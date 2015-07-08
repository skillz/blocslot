/*
 * This file is part of the Marmalade SDK Code Samples.
 *
 * (C) 2001-2012 Marmalade. All Rights Reserved.
 *
 * This source code is intended only as a supplement to the Marmalade SDK.
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "s3eKeyboard.h"
#include "s3ePointer.h"
#include "s3eDevice.h"
#include <time.h>

#include "Iw2D.h"
#include "IwResManager.h"

#include "game.h"
#include "rendering.h"
#include "localise.h"
#include "effects.h"
#include "titlescreen.h"

#include "SkillzSDK.h"

int32 SkillzDidFinishLaunching(void* systemData, void* appData)
{
    s3eDebugOutputString("Skillz integration: SkillzDidFinishLaunching");
    return S3E_RESULT_SUCCESS;
}

int32 TournamentWillStart(void* systemData, void*appData)
{
    s3eDebugOutputString("Skillz integration: TournamentWillStart");

    if (g_GameMode == MODE_TITLE)
    {
        g_GameMode = MODE_GAMEPLAY;
    }
    return S3E_RESULT_SUCCESS;
}

int32 SkillzWillExit(void* systemData, void* appData)
{
    s3eDebugOutputString("Skillz integration: SkillzWillExit");
    return S3E_RESULT_SUCCESS;
}

int32 ScoreReported(void* systemData, void* appData)
{
    s3eDebugOutputString("Skillz integration: ScoreReported");
    return S3E_RESULT_SUCCESS;
}

// Flag indicating whether the screen size or rotation has changed since the last call to SetupImages
bool g_ScreenSizeChanged = true;

// Flag indicating the game should be paused with a message informing the user that
// the game cannot be played in the current orientation.
bool g_ScreenTooSmall = false;

// Callback from S3E when the screen size or rotation changes
int32 ScreenSizeChangeCallback(void* systemData, void* userData)
{
    g_ScreenSizeChanged = true;
    return 0;
}

// Recalculate tile size when the screen size or rotation changes
// Calls 'SetupImages' to update the global pointers to materials.
// Also determines the size of the font to use
void UpdateScreenSize()
{
    if (g_ScreenSizeChanged)
    {
        g_ScreenSizeChanged = false;

        int x = Iw2DGetSurfaceWidth() / GAME_WIDTH;
        int y = Iw2DGetSurfaceHeight() / GAME_HEIGHT;
        int tempSize = MIN(x, y);

        g_ScreenTooSmall = (tempSize<12);

        if (tempSize < 16)
            tempSize = 12;
        else if (tempSize < 24)
            tempSize = 16;
        else if (tempSize < 32)
            tempSize = 24;
        else if (tempSize < 48)
            tempSize = 32;
        else
            tempSize = 48;

        if (g_TileSize != tempSize)
        {
            g_TileSize = tempSize;

            // Look up materials by name
            SetupImages(g_TileSize);

            // Choose font based on tile size
            if (g_TileSize <= 16)
                font = Iw2DCreateFontResource("font_small");
            else
                font = Iw2DCreateFontResource("font");

            //Set this font as the current one
            Iw2DSetFont(font);
        }
    }
}

// Draw the screen informing the user that the current orientation isn't supported
void DrawUnsupportedScreen()
{
    int displayWidth = Iw2DGetSurfaceWidth();
    int displayHeight = Iw2DGetSurfaceHeight();

    // Draw background
    DrawBlackBG(0, 0, displayWidth, displayHeight);

    // Draw message (centered, and automatically word wrapped)
    Iw2DDrawString(g_Localisation[ID_UNSUPPORTED_ORIENTATION],
        CIwSVec2(0,0),
        CIwSVec2((int16)displayWidth, (int16)displayHeight),
        IW_2D_FONT_ALIGN_CENTRE, IW_2D_FONT_ALIGN_CENTRE);
}



// Entry point
// The main() function initialises application data and required modules; it then
// enters a while() loop that exits only when the application throws a "quit" event. Each
// iteration of this loop performs a general "update" and "render" of the application. If the
// loop exits, application data is deleted and modules are terminated before exiting.
int main(int argc, char* argv[])
{
    // Initialisation of Studio modules
    Iw2DInit();         // Initialise support for rendering with the standard SW renderer

    // Lock orientation to portrait for Skillz integration
    s3eSurfaceSetInt(S3E_SURFACE_DEVICE_ORIENTATION_LOCK, S3E_SURFACE_PORTRAIT);

    IwResManagerInit();

    // Load all application data
    IwGetResManager()->LoadGroup("tiles.group");

    // Setup materials based on screen size
    UpdateScreenSize();

    // Register a callback so we are informed when the screen is rotated
    s3eSurfaceRegister(S3E_SURFACE_SCREENSIZE, ScreenSizeChangeCallback, NULL);

    g_EffectsManager = new EffectManager; // Manager for graphical effects

    PuzzleGame * game = new PuzzleGame;
    TitleScreen * title = new TitleScreen;

    // Register needed Skillz callbacks
    SkillzSDKRegister(SKILLZSDK_CALLBACK_SKILLZ_DID_LAUNCH, SkillzDidFinishLaunching, title);
    SkillzSDKRegister(SKILLZSDK_CALLBACK_TOURNAMENT_WILL_START_WITH_MATCH_DATA, TournamentWillStart, game);
    SkillzSDKRegister(SKILLZSDK_CALLBACK_REPORT_SCORE_HAS_COMPLETED, ScoreReported, NULL);
    SkillzSDKRegister(SKILLZSDK_CALLBACK_SKILLZ_WILL_EXIT, SkillzWillExit, title);

    uint32 timer = (uint32)s3eTimerGetMs();

    // Initialize game based on game id given by the Skillz Developer Portal
    SkillzInit("934", S3eSkillzSandbox);

    while (1)
    {
        s3eDeviceYield(0);

        // Check for user quit
        if (s3eDeviceCheckQuitRequest())
            break;

        // Check for screen resizing/rotation
        UpdateScreenSize();

        // Calculate the amount of time that's passed since last frame
        int delta = uint32(s3eTimerGetMs()) - timer;
        timer += delta;

        // Make sure the delta-time value is safe
        if (delta < 0)
            delta = 0;
        if (delta > 100)
            delta = 100;

        UpdateInput(delta);

        // Update and render
        if (g_ScreenTooSmall)
        {
            DrawUnsupportedScreen();
        }
        else if (g_GameMode == MODE_TITLE)
        {
            title->Update(delta);
            title->Render();

            // When starting a new game, make sure we reset the state
            if (g_GameMode == MODE_GAMEPLAY)
                game->Reset();
        }
        else
        {
            game->Update(delta);
            game->Render();
        }

        //Present the rendered surface to the screen
        Iw2DSurfaceShow();
    }

    // Delete objects and terminate systems
    s3eSurfaceUnRegister(S3E_SURFACE_SCREENSIZE, ScreenSizeChangeCallback);

    delete game;
    delete title;

    // Delete any left-over effects
    delete g_EffectsManager;

    CleanupImages();

    // Terminate system modules
    IwResManagerTerminate();
    Iw2DTerminate();

    return 0;
}
