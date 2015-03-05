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

#ifndef _EFFECTS_H
#define _EFFECTS_H

#include "IwArray.h"
#include "IwGeom.h"
#include "IwString.h"

// Note: the position of effects is stored as a fixed point position in the playing area.
// So IW_GEOM_ONE is the size of a tile.
// This allows the positions to remain consistant when the screen is rotated

// Base class for graphical effects
struct Effect
{
    virtual bool Update(int timeDeltaMs) = 0;
    virtual void Render() = 0;
    virtual ~Effect() {}
};

// A star-shaped particle, used when tiles explode
struct ExplosionFragment : public Effect
{
    CIwVec2 pos;
    CIwVec2 vel;
    int timer;
    int colour;

    ExplosionFragment(CIwVec2 const & startPos, CIwVec2 const & startVel, int _colour);
    bool Update(int timeDeltaMs);
    void Render();
};

// Floating text - drifts upwards, then disappears. Used to show the player how much score they're getting.
struct FloatText : public Effect
{
    CIwVec2 pos;
    int timer;
    CIwStringL text;

    FloatText(CIwVec2 const & startPos, const char * string);
    bool Update(int timeDeltaMs);
    void Render();
};

// Manager for graphical effects.
// All new instances of Effect should be added to a manager, which is then resposible for updating/rendering/deleting them.
struct EffectManager
{
    CIwArray<Effect*> effects;

    ~EffectManager();
    void Add(Effect* e) { effects.append(e); }
    void Clear();
    void Update(int timeDeltaMs);
    void Render();
};

extern EffectManager * g_EffectsManager;

#endif /* !_EFFECTS_H */
