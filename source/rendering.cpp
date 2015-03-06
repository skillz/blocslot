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

#include "rendering.h"
#include "Iw2D.h"
#include "IwResManager.h"

// Pointers to images used by the rendering code
CIw2DImage* backgroundImage;
CIw2DImage* starImage;
CIw2DImage* logoImage;
CIw2DImage* playImage;
CIw2DImage* tileImage[MAX_NUM_COLOURS];
CIw2DImage* touchscreenButtonImage[5];
CIw2DFont* font;


int g_TileSize = 0;

void CleanupImages()
{
    delete backgroundImage;
    delete starImage;
    delete logoImage;
    delete playImage;

    int i;
    for (i=0; i<MAX_NUM_COLOURS; i++)
        delete tileImage[i];

    for (i=0; i < 5; i++)
        delete touchscreenButtonImage[i];

    delete font;
}

// Lookup pointers to materials from IwResManager
// This is called again if the desired tile size changes (e.g. the user rotates the screen from portrait to landscape, so the old tile size is now too large to see the whole play area)
void SetupImages(int tileSize)
{
    CleanupImages();

    backgroundImage = Iw2DCreateImageResource("background");

    starImage = Iw2DCreateImageResource("star");
    logoImage = Iw2DCreateImageResource("logo");
    playImage = Iw2DCreateImageResource("play");

    const char* colourNames[] =
    {
        "",
        "green",
        "red",
        "lt_blue",
        "purple",
        "yellow",
    };

    int i;
    for (i=0; i<MAX_NUM_COLOURS; i++)
    {
        char tmp[32];
        sprintf(tmp, "tiles%d#%s", tileSize, colourNames[i]);
        tileImage[i] = Iw2DCreateImageResource(tmp);
    }

    touchscreenButtonImage[0] = Iw2DCreateImageResource("touchscreenRotL");
    touchscreenButtonImage[1] = Iw2DCreateImageResource("touchscreenRotR");
    touchscreenButtonImage[2] = Iw2DCreateImageResource("touchscreenMoveL");
    touchscreenButtonImage[3] = Iw2DCreateImageResource("touchscreenMoveDown");
    touchscreenButtonImage[4] = Iw2DCreateImageResource("touchscreenMoveR");
}

// Draw a star centered at the specified position (used for the effect when tiles explode)
void DrawSpriteCentered(CIw2DImage* img, int x, int y, int size)
{
    Iw2DDrawImage(img, CIwSVec2(x, y) - CIwSVec2(size/2, size/2), CIwSVec2(size, size));
}


// Draws a background by tiling the specified material to fill the specified area
void DrawBG(CIw2DImage* img, int x0, int y0, int w, int h)
{
    // This may well go over the edges specified to the right and bottom (since it only draws complete tiles)

    //Get Size of image
    int img_width  = img->GetWidth();
    int img_height = img->GetHeight();

        //Draw textured tiles
        for (int x=x0; x<w; x += img_width)
            for (int y=y0; y<h; y += img_height)
                Iw2DDrawImage(img, CIwSVec2(x, y));
}

// Draws a half-transparent black rectangle at the location
void DrawBlackBG(int x0, int y0, int w, int h)
{
    // Draw background for playing area
    Iw2DSetColour(0);
    Iw2DSetAlphaMode(IW_2D_ALPHA_HALF);
    Iw2DFillRect(CIwSVec2(x0, y0), CIwSVec2(w, h));
    Iw2DSetColour(0xffffffff);
    Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);
}

// Draw a tile
void DrawTile(int colour, int x, int y, int size, uint32 connectFlags)
{
    IwAssertMsg(APP, colour >= 0 && colour < MAX_NUM_COLOURS, ("Illegal colour for DrawTile (%d)", colour));

    int tileTypeX, tileTypeY;

    // Pick the appropriate part of the tile material to match the connections of this tile
    if (connectFlags & CONNECT_UP)
    {
        if (connectFlags & CONNECT_DOWN)
            tileTypeY = 2;
        else
            tileTypeY = 3;
    }
    else
    {
        if (connectFlags & CONNECT_DOWN)
            tileTypeY = 1;
        else
            tileTypeY = 0;
    }

    if (connectFlags & CONNECT_LEFT)
    {
        if (connectFlags & CONNECT_RIGHT)
            tileTypeX = 2;
        else
            tileTypeX = 3;
    }
    else
    {
        if (connectFlags & CONNECT_RIGHT)
            tileTypeX = 1;
        else
            tileTypeX = 0;
    }


    Iw2DDrawImageRegion(
        tileImage[colour],
        CIwSVec2(x, y),
        CIwSVec2(tileTypeX * g_TileSize, tileTypeY * g_TileSize),
        CIwSVec2(g_TileSize, g_TileSize)
        );
}
