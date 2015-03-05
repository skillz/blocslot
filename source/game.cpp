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

#include "game.h"
#include "effects.h"
#include "rendering.h"
#include "localise.h"

#include "Iw2D.h"
#include "s3eKeyboard.h"
#include "s3ePointer.h"

#include <time.h>

// Level progression settings
// Gravity is the number of milliseconds before the piece is automatically moved down one square
// Colours per level is the number of different colours used. This shouldn't decrease, else the player might be left with pieces they can't get rid of.
// piecesLevelBoundary is the cumulative number of pieces played to reach each level
// Currently, the level 0 settings are never used (levels go 1,2,3,4,5,6,7,8,9)
const int gravityPerLevel[10] = {400, 400, 290, 180, 300, 200, 120, 80,  60,  50,};
const int coloursPerLevel[10] = {  5,   5,   5,   5,   6,   6,   6,   6,   6,   6,};
const int piecesLevelBoundary[10] = {   0,   30,  60,  90,  120, 150, 180, 210, 250, -1,};

GameMode g_GameMode = MODE_TITLE;

int g_DrawTouchscreenButtons = 0;

CIwSVec2 g_RippleCentre;
int32 g_RippleDuration = 0;

//Iw2D supports an arbitrary post-transformation to be applied to all generated points/colours
void RippleFunc(CIwSVec2* v, CIwColour* c, int32 points)
{
    //Get the centre of the ripple in subpixels (8 subpixels per pixel)
    //Apply the transform matrix to the centre, so it's in the correct place
    CIwSVec2 rippleCentre = (CIwSVec2)(Iw2DGetTransformMatrix().TransformVec(g_RippleCentre) << 3);

    while (points--)
    {
        CIwSVec2 dirToCentre = *v - rippleCentre;
        int32 distToCentre = dirToCentre.GetLength() / g_TileSize;
        int32 rippleHeight = distToCentre*20 + (g_RippleDuration)*7 + IW_GEOM_ONE/2;
        if (distToCentre && rippleHeight >= 0)
        {
            dirToCentre.Normalise();
            int ofs = IW_FIXED_MUL(IW_FIXED_MUL(g_TileSize*(4<<3), g_RippleDuration*2), IwGeomSin(rippleHeight));
            *v += dirToCentre * ofs;//MAX(0, ofs);
        }
        c++;
        v++;
    }
}


//
// Input ////////////////////////////////////////////////////////////////////////
//

int input_x = 0;
int input_y = 0;
int input_rotation = 0;
int old_input_y = 0;

static int autoRepeatTimer = 0;
static int autoRepeatValue = 0;


void UpdateInput(int deltaTimeMs)
{
    s3ePointerUpdate();
    s3eKeyboardUpdate();

    old_input_y = input_y;

    input_y = input_x = input_rotation = 0;

    int xMovement = 0;
    if (s3eKeyboardGetState(s3eKeyAbsLeft) & S3E_KEY_STATE_DOWN)
        xMovement--;
    if (s3eKeyboardGetState(s3eKeyAbsRight) & S3E_KEY_STATE_DOWN)
        xMovement++;

    input_rotation = 0;
    if (s3eKeyboardGetState(s3eKeyAbsGameA) & S3E_KEY_STATE_PRESSED)
        input_rotation = -1;
    if (s3eKeyboardGetState(s3eKeyAbsGameB) & S3E_KEY_STATE_PRESSED)
        input_rotation = 1;
    if (s3eKeyboardGetState(s3eKeyAbsUp) & S3E_KEY_STATE_PRESSED)
        input_rotation = -1;

    if (s3eKeyboardGetState(s3eKeyAbsDown) & S3E_KEY_STATE_DOWN)
        input_y = 1;

    if (s3ePointerGetState(S3E_POINTER_BUTTON_SELECT) & S3E_POINTER_STATE_DOWN)
    {
        g_DrawTouchscreenButtons = 1;

        int displayWidth = Iw2DGetSurfaceWidth();
        int displayHeight = Iw2DGetSurfaceHeight();
        int x = s3ePointerGetX() * 3 / displayWidth;
        int y = s3ePointerGetY() * 4 / displayHeight;
        if (s3ePointerGetState(S3E_POINTER_BUTTON_SELECT) & S3E_POINTER_STATE_PRESSED)
        {
            if (x == 0 && y==2)
                input_rotation = -1;
            if (x == 2 && y==2)
                input_rotation = 1;
        }
        if (x == 0 && y==3)
            xMovement--;
        if (x == 2 && y==3)
            xMovement++;
        if (x == 1 && y==3)
            input_y = 1;
    }

    // Clamp xMovement to the range [-1,1]
    if (ABS(xMovement) > 1)
        xMovement = (xMovement < 0) ? -1 : 1;

    if (xMovement != autoRepeatValue || xMovement == 0)
    {
        // Reset auto-repeat timer
        autoRepeatValue = xMovement;
        autoRepeatTimer = 0;
        input_x = xMovement;
    }
    else
    {
        autoRepeatTimer += deltaTimeMs;
        if (autoRepeatTimer >= 200)
        {
            // Faster auto-repeat after the first repeat
            autoRepeatTimer = 100;
            input_x = xMovement;
        }
        else
            input_x = 0;
    }
}

void DrawTouchscreenButtons()
{
    int displayWidth = Iw2DGetSurfaceWidth();
    int displayHeight = Iw2DGetSurfaceHeight();

    int size = MIN(displayWidth/3, displayHeight/4) - 8;
    if (size < 64)
        size = 32;
    else
        size = 64;

    Iw2DSetColour(0xff646464);
    Iw2DSetAlphaMode(IW_2D_ALPHA_ADD);
    DrawSpriteCentered(touchscreenButtonImage[0], size/2, displayHeight*5/8, size);
    DrawSpriteCentered(touchscreenButtonImage[1], displayWidth-size/2, displayHeight*5/8, size);
    DrawSpriteCentered(touchscreenButtonImage[2], size/2, displayHeight-size/2, size);
    DrawSpriteCentered(touchscreenButtonImage[3], displayWidth/2, displayHeight-size/2, size);
    DrawSpriteCentered(touchscreenButtonImage[4], displayWidth-size/2, displayHeight-size/2, size);
    Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);
    Iw2DSetColour(0xffffffff);
}

//
// Tile class ////////////////////////////////////////////////////////////////////////
//

// Rotate the 'connect' bitfield by the requested amount (1=90 degree rotation)
void Tile::RotateConnections(int r)
{
    IwAssertMsg(APP, (r & 3) == r, ("Illegal rotation in Tile::RotateConnections"));
    connect = (connect >> r) | (connect << (4-r));
    connect &= 15;
}


// Clear the tile
void Tile::Clear()
{
    connect = 0;
    col = 0;
    groupId = -1;
}



//
// Grid class ////////////////////////////////////////////////////////////////////////
//

bool Grid::RowEmpty(int y) const
{
    for (int x=0; x<width; x++)
    {
        if (Get(x,y))
            return false;
    }

    return true;
}

void Grid::Resize(int newWidth, int newHeight)
{
    if (width != newWidth || height != newHeight)
    {
        width = newWidth;
        height = newHeight;
        delete [] tile;
        tile = new Tile[width*height];
    }
}

Grid & Grid::operator = (Grid const & g)
{
    Resize(g.width, g.height);

    memcpy(tile, g.tile, width*height*sizeof(tile[0]));

    numRotations = g.numRotations;
    currentRotation = g.currentRotation;

    return *this;
}


void Grid::Clear()
{
    currentRotation = 0;
    numRotations = 4;
    for (int i=0; i<width*height; i++)
        tile[i].Clear();
}


Tile & Grid::Get(int x, int y)
{
    IwAssertMsg(APP, Valid(x,y), ("Coordinate out of range for Grid (%d,%d)", x, y));
    return tile[x + y*width];
}


const Tile & Grid::Get(int x, int y) const
{
    IwAssertMsg(APP, Valid(x,y), ("Coordinate out of range for Grid (%d,%d)", x, y));
    return tile[x + y*width];
}


void Grid::SetTile(int x, int y, int col)
{
    Get(x,y).SetCol(col);
}


bool Grid::Valid(int x, int y) const
{
    return x>=0 && y>=0 && x<width && y<height;
}


void Grid::Render(int rx, int ry) const
{
    for (int x=0; x<width; x++)
    {
        for (int y=0; y<height; y++)
        {
            Tile const & t = Get(x,y);
            if (t)
            {
                DrawTile(
                    t.col-1,
                    x*g_TileSize + rx,
                    y*g_TileSize + ry,
                    g_TileSize,
                    t.connect
                    );
            }
        }
    }
}


void Grid::Rotate(int r)
{
    // We only need to support square grids here.
    IwAssertMsg(APP, width == height, ("Only square Grids can be rotated"));

    // Given we rotate around the center of a square, it looks nicer if we don't allow all 4 rotations of some pieces.
    // E.g. the 2x2 square activePiece will appear to slide around when rotated.

    if (numRotations <= 1 || r == 0)
        return;

    r = (currentRotation + 4 + r) % numRotations - currentRotation;
    r = r & 3;

    currentRotation = (currentRotation + r) % numRotations;

    // Rotate the connection information (this is done by rotating the bit field)
    if (r)
        for (int i=0; i<width*height; i++)
            tile[i].RotateConnections(r);

    while (r--)
    {
        // Rotate the arrangement of tiles through 90 degrees
        for (int x=0; x<width/2; x++)
        {
            for (int y=0; y<(height+1)/2; y++)
            {
                Tile tmp = Get(x,y);
                Get(x,y) = Get(y,width-1-x);
                Get(y,width-1-x) = Get(width-1-x,width-1-y);
                Get(width-1-x,width-1-y) = Get(width-1-y,x);
                Get(width-1-y,x) = tmp;
            }
        }
    }
}


void Grid::AddToWorld(Grid& g, int offsetX, int offsetY) const
{
    // Copy any non-empty tiles from this grid into the specified target (with offset)
    for (int x=0; x<width; x++)
    {
        for (int y=0; y<height; y++)
        {
            if (Get(x,y))
                g.Get(x+offsetX, y+offsetY) = Get(x,y);
        }
    }
}


bool Grid::Collide(Grid const & g, int ox, int oy) const
{
    // Iterate over all non-empty tiles inside this Grid, and check if they overlap with any tiles in the specified target grid
    // Treats non-empty tiles of this grid being outside the target grid as a collision.
    for (int x=0; x<width; x++)
        for (int y=0; y<height; y++)
        {
            if (Get(x,y))
            {
                int x1 = x+ox;
                int y1 = y+oy;
                if (!g.Valid(x1,y1))
                    return true;
                if (g.Get(x1,y1))
                    return true;
            }
        }

    return false;
}


int Grid::UpdateConnections()
{
    // Links all similarly coloured tiles together.
    // Returns the number of extra links added

    int count = 0;
    for (int y=0; y<height; y++)
        for (int x=0; x<width; x++)
            count += UpdateTileConnections(x,y);

    // Divide count by 2, since the we count each connection twice
    return count / 2;
}


int Grid::UpdateTileConnections(int x, int y)
{
    // Links the specified tile to any adjacent tiles of the same colour
    // Returns the number of extra links added

    Tile& t = Get(x,y);
    if (t)
    {
        uint32 connect = 0;
        if (x>0 && t.JoinsTo(Get(x-1,y)))
            connect |= CONNECT_LEFT;
        if (x<width-1 && t.JoinsTo(Get(x+1,y)))
            connect |= CONNECT_RIGHT;
        if (y>0 && t.JoinsTo(Get(x,y-1)))
            connect |= CONNECT_UP;
        if (y<height-1 && t.JoinsTo(Get(x,y+1)))
            connect |= CONNECT_DOWN;

        if (t.connect != connect)
        {
            int extraConnections = connect & ~t.connect;
            t.connect = connect;

            // Count the number of new connections added
            int count = 0;
            for (; extraConnections; extraConnections>>=1)
                if (extraConnections & 1)
                    count++;

            return count;
        }
    }

    return 0;
}


void Grid::FloodFill(int x, int y, int id)
{
    // Recursively find all adjacent tiles of the same colour, and set their id.
    // Also counts the number of tiles in the group (stored in the member variable 'groupSizes')
    // This assumes the ids have been set to 0 first.

    Tile& t = Get(x,y);

    groupSizes[id]++;
    t.groupId = id;

    if (x>0 && t.CanFloodFillTo(Get(x-1,y)))
        FloodFill(x-1, y, id);
    if (x<width-1 && t.CanFloodFillTo(Get(x+1,y)))
        FloodFill(x+1, y, id);
    if (y>0 && t.CanFloodFillTo(Get(x,y-1)))
        FloodFill(x, y-1, id);
    if (y<height-1 && t.CanFloodFillTo(Get(x,y+1)))
        FloodFill(x, y+1, id);
}


void Grid::CreateGroups()
{
    // Clear group information
    for (int i=0; i<width*height; i++)
        tile[i].groupId = -1;

    groupSizes.clear();

    // Build new group information by floodfilling
    for (int x=0; x<width; x++)
        for (int y=0; y<height; y++)
            if (Get(x,y) && Get(x,y).groupId == -1)
            {
                int id = groupSizes.size();
                groupSizes.append(0);
                FloodFill(x, y, id);
            }
}


bool Grid::MakeFall()
{
    // Move all unsupported pieces downwards.
    // Returns true if anything moved.
    // This assumes CreateGroups has already been called

    bool falling = false;

    CIwArray<int> supported;
    supported.resize(groupSizes.size());
    for (uint32 i=0; i<supported.size(); i++)
        supported[i] = 0;

    while (1)
    {
        bool changed = false;

        // Iterate from the bottom towards the top, finding unsupported groups which will need to fall
        for (int y=height-1; y>=0; y--)
        {
            for (int x=0; x<width; x++)
            {
                // If the this tile's group isn't supported, check if it should be.
                // (Either because it's at the bottom of the playing area, or it's resting on another supported group)
                if (Get(x,y) && !supported[Get(x,y).groupId])
                {
                    if (y == height-1 ||
                        (Get(x, y + 1) && supported[Get(x, y + 1).groupId]))
                    {
                        supported[Get(x, y).groupId] = 1;
                        changed = true;
                    }
                }
            }
        }

        // Keep re-scanning the tiles until we don't find any more supported ones
        if (!changed)
            break;
    }

    // Iterate from the bottom towards the top, moving unsupported groups downwards
    for (int y=height-1; y>=0; y--)
        for (int x=0; x<width; x++)
            if (Get(x,y) && !supported[Get(x,y).groupId])
            {
                Get(x,y+1) = Get(x,y);
                Get(x,y).Clear();
                falling = true;
            }

    return falling;
}


int Grid::CheckForExplosions(int criticalMass, CIwVec2 & centre)
{
    // Checks for groups of like-coloured tiles big enough to be removed.
    // Returns the total number of tiles removed
    // Note: this will only remove one group at a time, for the benefit of the scoring system

    CreateGroups();

    centre.x = centre.y = 0;

    int explosions = 0;
    int explodeGroup = -1;
    for (int x=0; x<width; x++)
        for (int y=0; y<height; y++)
            if (Get(x,y) && groupSizes[Get(x,y).groupId]>=criticalMass)
            {
                if (explodeGroup == -1 || explodeGroup == Get(x, y).groupId)
                {
                    explosions++;
                    explodeGroup = Get(x, y).groupId;

                    centre.x += x;
                    centre.y += y;
                }
            }

    if (explosions)
    {
        // Take average of tile positions
        centre.x = centre.x / explosions;
        centre.y = centre.y / explosions;

        g_RippleDuration = 500;

        //Remember the ripple centre for the post transform callback
        g_RippleCentre = (centre * IW_FIXED(g_TileSize));


        // convert into fixed point, and move to the middle of the tile
        centre = (centre << IW_GEOM_POINT) + CIwVec2(IW_FIXED(0.5), IW_FIXED(0.5));

        // Create fragments, and clear tiles
        for (int x=0; x<width; x++)
            for (int y=0; y<height; y++)
                if (Get(x,y).groupId == explodeGroup)
                {
                    CIwVec2 p = (CIwVec2(x,y) << IW_GEOM_POINT) + CIwVec2(IW_FIXED(0.5), IW_FIXED(0.5));
                    CIwVec2 v = (p - centre);
                    v.x += rand() % 2000 - 1000;
                    v.y += rand() % 2000 - 1000;
                    v.Normalise();

                    g_EffectsManager->Add(new ExplosionFragment(p, v * IW_FIXED(27), Get(x,y).col));

                    g_EffectsManager->Add(new ExplosionFragment(p, v * IW_FIXED(60), Get(x,y).col));

                    Get(x,y).Clear();
                }
    }

    return explosions;
}



//
// PuzzleGame class ////////////////////////////////////////////////////////////////////////
//

PuzzleGame::PuzzleGame()
{
    grid.Resize(GAME_WIDTH, GAME_HEIGHT);
    Reset();
}

// Reset game (used when a new game starts)
void PuzzleGame::Reset()
{
    g_EffectsManager->Clear();

    // Seed random number generator
    srand(time(NULL));

    score = 0;
    totalPieceCount = 0;
    level = 1;


    grid.Clear();
    CreateRandomPiece(nextPiece, coloursPerLevel[level]);
    NewPiece(); // Makes the initially created piece active, and creates a new "next piece"
}

void PuzzleGame::Render()
{
    int displayWidth = Iw2DGetSurfaceWidth();
    int displayHeight = Iw2DGetSurfaceHeight();

    bool previewRight = true;
    bool previewTop = false;

    int centeringWidth = (grid.width + 4) * g_TileSize + g_TileSize/2;
    int centeringHeight = (grid.height + 0) * g_TileSize;
    if (centeringWidth > displayWidth)
    {
        previewRight = false;
        centeringWidth = grid.width * g_TileSize;
        centeringHeight = (grid.height + 3) * g_TileSize;

        if (centeringHeight > displayHeight)
        {
            centeringHeight = (grid.height + 0) * g_TileSize;
        }
        else
            previewTop = true;
    }

#ifndef IW_MKF_IW2D_LITE
    if (g_RippleDuration)
        Iw2DSetPostTransformFn(RippleFunc);
#endif


    // Draw overall background
    DrawBG(backgroundImage, 0, 0, displayWidth, displayHeight);

    // Set the 2D transform to have a translation to the top-left corner of where we want
    // the main game area drawn. This reduces the complexity of the actual render functions
    CIwMat2D trans = CIwMat2D::g_Identity;
    trans.t.x = int16( displayWidth/2  - centeringWidth/2 );
    trans.t.y = int16( displayHeight/2 - centeringHeight/2 );
    if (previewTop)
        trans.t.y += 3 * g_TileSize;
    Iw2DSetTransformMatrix(trans);

    DrawBlackBG(0, 0, g_TileSize*grid.width, g_TileSize*grid.height);

    // Draw playing area and active piece
    grid.Render(0, 0);
    if (mode == MODE_ACTIVE_PIECE)
        activePiece.Render(piecePos.x*g_TileSize, piecePos.y*g_TileSize);

    // Draw next piece indicator
    if (mode != MODE_GAME_OVER)
    {
        if (previewRight)
        {
            int nextPieceX = grid.width*g_TileSize + g_TileSize/2;
            nextPiece.Render(nextPieceX, 0);
        }
        if (previewTop)
        {
            nextPiece.Render(g_TileSize*2, g_TileSize*-4);
        }
    }

    // Draw effects
    g_EffectsManager->Render();

    // Draw player's score
    char scoreString[32];
    sprintf(scoreString, "%s %d\n%s %d", g_Localisation[ID_SCORE], score, g_Localisation[ID_LEVEL], level);
    Iw2DDrawString(scoreString,
        CIwSVec2(0,0), CIwSVec2((int16)displayWidth, (int16)displayHeight),
        IW_2D_FONT_ALIGN_LEFT, IW_2D_FONT_ALIGN_TOP);

    if (mode == MODE_GAME_OVER)
    {
        Iw2DDrawString(g_Localisation[ID_GAME_OVER],
            CIwSVec2(0,0), CIwSVec2(g_TileSize*grid.width,g_TileSize*grid.height),
            IW_2D_FONT_ALIGN_CENTRE, IW_2D_FONT_ALIGN_CENTRE);
    }

    // Reset screen space origin
    Iw2DSetTransformMatrix(CIwMat2D::g_Identity);

#ifndef IW_MKF_IW2D_LITE
    if (g_RippleDuration)
        Iw2DSetPostTransformFn(NULL);
#endif

    if (g_DrawTouchscreenButtons && mode != MODE_GAME_OVER)
    {
        DrawTouchscreenButtons();
    }
}

// Attempt to move the active activePiece by the specified amount.
// If the activePiece cannot be moved in the direction requested, it is not moved.
bool PuzzleGame::MovePiece(int x, int y, int rotation)
{
    if (x == 0 && y == 0 && rotation == 0)
        return true;

    activePiece.Rotate(rotation);

    if (!activePiece.Collide(grid, piecePos.x + x, piecePos.y + y))
    {
        piecePos.x += x;
        piecePos.y += y;
        return true;
    }

    // Target position is not valid, so revert to original rotation
    activePiece.Rotate(-rotation);
    return false;
}

// Create a new piece with random shape and colour
void PuzzleGame::CreateRandomPiece(Grid & newPiece, int numColours)
{
    newPiece.Resize(5,5);
    newPiece.Clear();

    int col = rand() % numColours + 1;
    switch (rand() % 7)
    {
    case 0:
        // 2x2 Square
        newPiece.SetTile(1,1,col);
        newPiece.SetTile(1,2,col);
        newPiece.SetTile(2,1,col);
        newPiece.SetTile(2,2,col);
        newPiece.numRotations = 1;
        break;
    case 1:
        // 1x4 Long thin piece
        newPiece.SetTile(0,2,col);
        newPiece.SetTile(1,2,col);
        newPiece.SetTile(2,2,col);
        newPiece.SetTile(3,2,col);
        newPiece.numRotations = 2;
        break;
    case 2:
        // 'Z' shaped piece
        newPiece.SetTile(1,1,col);
        newPiece.SetTile(2,1,col);
        newPiece.SetTile(2,2,col);
        newPiece.SetTile(3,2,col);
        newPiece.numRotations = 2;
        break;
    case 3:
        // 'S' shaped piece
        newPiece.SetTile(1,2,col);
        newPiece.SetTile(2,2,col);
        newPiece.SetTile(2,1,col);
        newPiece.SetTile(3,1,col);
        newPiece.numRotations = 2;
        break;
    case 4:
        // 'T' shaped piece
        newPiece.SetTile(2,1,col);
        newPiece.SetTile(1,2,col);
        newPiece.SetTile(2,2,col);
        newPiece.SetTile(3,2,col);
        newPiece.numRotations = 4;
        break;
    case 5:
        // Backwards 'L' shaped piece
        newPiece.SetTile(1,1,col);
        newPiece.SetTile(1,2,col);
        newPiece.SetTile(2,2,col);
        newPiece.SetTile(3,2,col);
        newPiece.numRotations = 4;
        break;
    case 6:
        // 'L' shaped piece
        newPiece.SetTile(3,1,col);
        newPiece.SetTile(1,2,col);
        newPiece.SetTile(2,2,col);
        newPiece.SetTile(3,2,col);
        newPiece.numRotations = 4;
        break;
    }

    // Link tiles together
    newPiece.UpdateConnections();
}

void PuzzleGame::NewPiece()
{
    activePiece = nextPiece;

    piecePos.x = (grid.width - activePiece.width)/2;
    piecePos.y = 0;

    // Move piece upwards so that it touches the top of the play area
    for (int i=0; i<5; i++)
    {
        if (activePiece.RowEmpty(i))
            piecePos.y--;
        else
            break;
    }

    // Count number of pieces created, and increase the difficulty level if necessary
    totalPieceCount++;
    if (level < 9 && totalPieceCount > piecesLevelBoundary[level])
    {
        level++;
    }

    // Create a new 'next piece'
    CreateRandomPiece(nextPiece, coloursPerLevel[level]);

    // Reset piece-related variables for new piece
    timer = 0;
    landTimer = 0;
    slideDirection = 0;
    multiplier = 1;

    if (activePiece.Collide(grid, piecePos.x, piecePos.y))
    {
        // Can't spawn new piece without it overlapping existing tiles.
        // Game over!
        mode = MODE_GAME_OVER;
    }
    else
    {
        mode = MODE_ACTIVE_PIECE;
    }
}

// Add the active piece to the world
void PuzzleGame::LandPiece()
{
    activePiece.AddToWorld(grid, piecePos.x, piecePos.y);
    activePiece.Clear();

    int c = grid.UpdateConnections();

    grid.CreateGroups();

    mode = MODE_EXPLODING;
    timer = 0;

    // Score points for adding another piece to the world
    // More points for pieces which fit together nicely (measured by the number of new connections created)
    score += c * c * 10 + 10;
}

// Check for explosions and give score for them.
// Returns true if something exploded
bool PuzzleGame::Explode()
{
    CIwVec2 explosionCenter;
    int c = grid.CheckForExplosions(EXPLODE_THRESHOLD, explosionCenter);

    if (c == 0)
    {
        return false;
    }
    else
    {
        // Things blew up - give score reward
        // Extra points for blowing up larger groups of tiles
        int extra = (c - EXPLODE_THRESHOLD) / 4;
        int scoreAdd = 300 + extra * 100 + extra * extra * 100;

        score += scoreAdd * multiplier;

        // Create a floating text object to inform the user of the point gain
        char scoreString[32];
        if (multiplier>1)
            sprintf(scoreString, "%dx%d", scoreAdd, multiplier);
        else
            sprintf(scoreString, "%d", scoreAdd);

        g_EffectsManager->Add(new FloatText(explosionCenter, scoreString));


        // Increase multiplier so chain reactions are worth more points
        if (multiplier < 64)
            multiplier *= 2;

        // Delay next logical update to give the player a bit more time to see chain reactions
        timer -= 150;
        return true;
    }
}

// Attempt to apply the specified user input.
// The parameters are references so this function can be called repeatedly without the piece going too far.
void PuzzleGame::ApplyUserInput(int & xMovement, int & rotation)
{
    int dir = xMovement<0 ? -1 : xMovement>0 ? 1 : 0;
    while (xMovement != 0 && MovePiece(dir, 0, 0))
    {
        // Reset land timer, to ensure user can slide pieces along the floor when necessary
        // Note: this only applies to moving it repeatedly in one direction to avoid the piece never landing.
        if (slideDirection == xMovement || slideDirection == 0)
        {
            slideDirection = xMovement;
            landTimer = 0;
        }
        xMovement -= dir;
    }

    if (rotation)
    {
        // If we can't move the piece and the player is trying to rotate it, try shifting it around.
        // This makes things more responsive for the player.

        if (MovePiece(0, 0, rotation))
            rotation = 0;
        else if (MovePiece(1, 0, rotation))
            rotation = 0;
        else if (MovePiece(-1, 0, rotation))
            rotation = 0;
        else if (MovePiece(0, 1, rotation))
            rotation = 0;
        else if (MovePiece(0, 2, rotation))
            rotation = 0;
    }
}

void PuzzleGame::Update(int deltaTimeMs)
{
    g_EffectsManager->Update(deltaTimeMs);

    //countdown ripple effect
    g_RippleDuration -= deltaTimeMs;
    if (g_RippleDuration < 0)
        g_RippleDuration = 0;

    // Accumulate time
    timer += deltaTimeMs;

    if (s3eKeyboardGetState(s3eKeyR) & S3E_KEY_STATE_PRESSED)
    {
        // Reset gameplay (for testing)
        Reset();
    }

    if (s3eKeyboardGetState(s3eKeyL) & S3E_KEY_STATE_PRESSED)
    {
        // Increase level (for testing)
        if (level < 9)
            level++;
    }

    if (s3eKeyboardGetState(s3eKeyAbsBSK) & S3E_KEY_STATE_PRESSED)
    {
        // Go back to title page
        g_GameMode = MODE_TITLE;
        // No point updating the game
        return;
    }

    if (mode == MODE_GAME_OVER)
    {
        // Wait briefly before accepting input so the player doesn't accidentally skip the game over screen.
        if (timer > 500)
        {
            if ((s3eKeyboardGetState(s3eKeyAbsASK) & S3E_KEY_STATE_PRESSED)
              || (s3ePointerGetState(S3E_POINTER_BUTTON_SELECT) & S3E_POINTER_STATE_PRESSED) )
            {
                g_GameMode = MODE_TITLE;
            }
        }
    }
    else if (mode == MODE_EXPLODING)
    {
        if (timer >= 100)
        {
            timer = 0;

            if (!Explode())
            {
                // Nothing more to blow up; check for pieces which used to be supported falling

                if (!grid.MakeFall())
                {
                    // Nothing falling; create a new piece
                    NewPiece();
                }
                else
                {
                    mode = MODE_FALLING;
                }
            }
        }
    }
    else if (mode == MODE_FALLING)
    {
        if (timer >= 5*20)
        {
            timer = 0;

            if (!grid.MakeFall())
            {
                // Things have landed; update connections and check for explosions
                grid.UpdateConnections();

                if (!Explode())
                {
                    // Nothing blew up; create a new piece
                    NewPiece();
                }
                else
                {
                    mode = MODE_EXPLODING;
                }
            }
        }
    }
    else if (mode == MODE_ACTIVE_PIECE)
    {
        int rotation = input_rotation;
        int xMovement = input_x;
        int down = (input_y>0);

        ApplyUserInput(xMovement, rotation);

        int gravityTime = gravityPerLevel[level];

        int downTime = MIN(gravityTime/2, 80);

        if (input_y>0 && old_input_y<=0)
            timer = downTime;

        while (timer > gravityTime || (down && timer >= downTime))
        {
            if (!MovePiece(0,1,0))
            {
                // Can't move piece downwards, so make it land after a while
                // This is timed using 'landTimer' to give the player more oppurtunity to slide the piece after it touches the floor.
                landTimer += deltaTimeMs;
                if (down || landTimer>200)
                {
                    LandPiece();
                }
                else
                {
                    // Don't accumulate too much time in the timer
                    timer = MIN(timer, gravityTime);
                }

                // Don't loop, since we're not cancelling the timer
                break;
            }
            else
            {
                if (down)
                    timer -= downTime;
                else
                    timer -= gravityTime;

                // The piece is falling, so reset the landing timer
                landTimer = 0;
                slideDirection = 0;
            }

            // Ensure the user can fit pieces into small gaps even if the piece fall speed is very high
            ApplyUserInput(xMovement, rotation);
        }
    }
}
