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

#ifndef _RENDERING_H
#define _RENDERING_H

#include "game.h"

// Foward declarations
class CIw2DImage;
class CIw2DFont;

extern CIw2DImage* backgroundImage;
extern CIw2DImage* starImage;
extern CIw2DImage* logoImage;
extern CIw2DImage* playImage;
extern CIw2DImage* tileImage[MAX_NUM_COLOURS];
extern CIw2DImage* touchscreenButtonImage[5];
extern CIw2DFont* font;

void CleanupImages();
void SetupImages(int tileSize);
void DrawSpriteCentered(CIw2DImage* img, int x, int y, int size);
void DrawBG(CIw2DImage* img, int x0, int y0, int w, int h);
void DrawBlackBG(int x0, int y0, int w, int h);
void DrawTile(int colour, int x, int y, int size, uint32 connectFlags);

// Global variable used by the rendering functions to scale graphics.
extern int g_TileSize;

#endif /* !_RENDERING_H */
