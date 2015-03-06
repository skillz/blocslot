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

#ifndef _TITLESCREEN_H
#define _TITLESCREEN_H

#include "s3eKeyboard.h"
#include "s3ePointer.h"
#include "s3eDevice.h"

#include "SkillzSDK.h"

// Simple class representing the title screen. Handles user input and rendering whilst on the title screen
class TitleScreen
{
private:
    int timer;
    int playX;
    int playY;

public:
    TitleScreen()
    {
        timer = 0;
        playX = 0;
        playY = 0;
    }

    void Update(int deltaTimeMs)
    {
        timer += deltaTimeMs;

        if ((s3eKeyboardGetState(s3eKeyAbsBSK) & S3E_KEY_STATE_PRESSED))
        {
            // Quit
            static bool disableExit =
                s3eDeviceGetInt(S3E_DEVICE_OS) == S3E_OS_ID_WS8 ||
                s3eDeviceGetInt(S3E_DEVICE_OS) == S3E_OS_ID_WS81;

            if (!disableExit)
            {
                s3eDeviceRequestQuit();
            }
        }

        bool playbuttonPressed = false;
        if (s3ePointerGetState(S3E_POINTER_BUTTON_SELECT) & S3E_POINTER_STATE_PRESSED)
        {
            int x = s3ePointerGetX();
            int y = s3ePointerGetY();
            int width = playImage->GetWidth();
            int height = playImage->GetHeight();

            if ((x >= playX & x <= playX + width)
                & (y >= playY & y <= playY + height)
                )
            {
                playbuttonPressed = true;
            }
        }

        if ((s3eKeyboardGetState(s3eKeyAbsGameA) & S3E_KEY_STATE_PRESSED)
         || (s3eKeyboardGetState(s3eKeyAbsASK) & S3E_KEY_STATE_PRESSED)
         || playbuttonPressed
           )
        {
            // Launch Skillz
            SkillzLaunch(S3eSkillzPortrait);
        }
    }

    void Render()
    {
        int displayWidth  = Iw2DGetSurfaceWidth();
        int displayHeight = Iw2DGetSurfaceHeight();

        // Draw background, scrolling diagonally
        int scrollPosition = (timer >> 4) % 128;

        DrawBG(backgroundImage, -scrollPosition, -scrollPosition, displayWidth+128, displayHeight+128);

        // Draw title logo centered on screen
        int x = displayWidth/2 - logoImage->GetWidth()/2;
        int y = 0;

        if (displayHeight > 176)
            y = MIN(32, (displayHeight - 176)/2);

        // Draw a sprite at the specified position, using the specified material.
        // The size of the sprite is taken from the size of the texture.
        Iw2DDrawImage(logoImage, CIwSVec2(x, y));

        // Draw a play button to start Skillz
        playX = x;
        playY = y + (displayHeight / 2);
        Iw2DDrawImage(playImage, CIwSVec2(playX, playY));
    }
};

#endif /* !_TITLESCREEN_H */
