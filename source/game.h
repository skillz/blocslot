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
#ifndef _GAME_H
#define _GAME_H

#include "IwArray.h"
#include "IwGeom.h"

enum GameMode
{
    MODE_TITLE,
    MODE_GAMEPLAY,
};

extern GameMode g_GameMode;

// Bitfield used for remembering which directions from a tile contain a connected tile
enum ConnectFlags
{
    CONNECT_UP    = 1<<0,
    CONNECT_LEFT  = 1<<1,
    CONNECT_DOWN  = 1<<2,
    CONNECT_RIGHT = 1<<3,
};

// Width and height of the playing area
#define GAME_WIDTH  10
#define GAME_HEIGHT 16

// Number of different colour tiles that are used
#define MAX_NUM_COLOURS 6

// Number of adjacent tiles of the same colour needed before they explode
#define EXPLODE_THRESHOLD 12

void UpdateInput(int deltaTimeMs);

// Class representing a single square in the game.
struct Tile
{
    int col;        // Colour of this tile (0 = empty)
    int groupId;    // ID of group this tile is in (used by the Grid class)
    uint32 connect; // Bitfield of which sides of this tile connect to squares of the same colour

    Tile()
    {
        Clear();
    }

    void Clear();

    void SetCol(int c)
    {
        col = c;
    }

    operator bool() const
    {
        return col != 0;
    }

    bool JoinsTo(Tile const & other) const
    {
        return col == other.col;
    }

    bool CanFloodFillTo(Tile const & other) const
    {
        return col == other.col && other.groupId == -1;
    }

    void RotateConnections(int r);
};

// Container class for holding a 2 dimensional array of tiles
// This is used both for the main play area and the individual pieces before they are added to the main play area
struct Grid
{
    int width,height;
    Tile *tile;
    CIwArray <int> groupSizes;
    int numRotations;
    int currentRotation;

    void FloodFill(int x, int y, int id);

public:

    Grid() : width(0), height(0), tile(NULL), numRotations(4), currentRotation(0)
    {
    }

    Grid(Grid const & g) : width(0), height(0), tile(NULL), numRotations(4), currentRotation(0)
    {
        *this = g;
    }

    ~Grid()
    {
        delete [] tile;
    }

    bool RowEmpty(int y) const;
    void Resize(int newWidth, int newHeight);
    Grid & operator = (Grid const & g);
    void Clear();
    Tile & Get(int x, int y);
    const Tile & Get(int x, int y) const;
    void SetTile(int x, int y, int col);
    bool Valid(int x, int y) const;
    void Render(int rx, int ry) const;
    void Rotate(int r);
    void AddToWorld(Grid& g, int offsetX, int offsetY) const;
    bool Collide(Grid const & g, int ox, int oy) const;
    int UpdateConnections();
    int UpdateTileConnections(int x, int y);
    void CreateGroups();
    bool MakeFall();
    int CheckForExplosions(int criticalMass, CIwVec2 & centre);
};


struct PuzzleGame
{
    enum UpdateMode
    {
        MODE_ACTIVE_PIECE,  // There is an active piece under player control
        MODE_FALLING,       // Things are falling (following an explosion) without player control
        MODE_EXPLODING,     // Things are exploding (without player control)
        MODE_GAME_OVER,     // The game is over
    };

    Grid grid;          // Main play area
    Grid activePiece;   // Active piece (i.e. the one which the user can move)
    Grid nextPiece;     // Next piece
    CIwVec2 piecePos;   // Position of active piece in the main play area
    int timer;          // Millisecond accumulator used for speed regulation
    UpdateMode mode;    // State of simulation
    int landTimer;      // Millisecond accumulator used for measuring the time between the piece touching the ground and actually finally landing.
    int slideDirection; // Which way the user is sliding the piece along the ground.
    int score;          // Player's score
    int level;          // Difficulty level. Starts at 1, and goes up to 9
    int totalPieceCount;    // Counter of pieces used. Used to determine when to increase the difficulty level.
    int multiplier;         // Score multiplier. Used to reward combos. Reset to 1 whenever a new piece is added

    PuzzleGame();
    void Reset();
    void Render();
    bool MovePiece(int x, int y, int rotation);
    void NewPiece();
    void LandPiece();
    bool Explode();
    void ApplyUserInput(int & xMovement, int & rotation);
    void Update(int deltaTimeMs);

    static void CreateRandomPiece(Grid & newPiece, int numColours);
};

#endif /* !_GAME_H */
