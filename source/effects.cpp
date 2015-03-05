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

#include "effects.h"
#include "rendering.h"

#include "Iw2D.h"

EffectManager * g_EffectsManager = NULL;

//
// FloatText class ////////////////////////////////////////////////////////////////////////
//

FloatText::FloatText(CIwVec2 const & startPos, const char * string)
{
    timer = 0;
    pos = startPos;
    text = string;
}

bool FloatText::Update(int timeDeltaMs)
{
    // Slide upwards
    timer += timeDeltaMs;
    pos.y -= timeDeltaMs * 15;

    // The text disappears after 2 seconds
    return timer<2000;
}

void FloatText::Render()
{
    // Convert position to pixels
    // Set the rectangle for font rendering to 400 pixels square (arbitrary) centered on our position
    Iw2DDrawString(text.c_str(),
        CIwSVec2((int16)IW_FIXED_MUL(pos.x, g_TileSize)-200,(int16)IW_FIXED_MUL(pos.y, g_TileSize)-200),
        CIwSVec2(400,400),
        IW_2D_FONT_ALIGN_CENTRE, IW_2D_FONT_ALIGN_CENTRE);
}


//
// ExplosionFragment class ////////////////////////////////////////////////////////////////////////
//

ExplosionFragment::ExplosionFragment(CIwVec2 const & startPos, CIwVec2 const & startVel, int _colour)
{
    colour = _colour;
    timer = rand() % 200;
    pos = startPos;
    vel = startVel;

    vel.y -= 100000;
}
bool ExplosionFragment::Update(int timeDeltaMs)
{
    timer += timeDeltaMs;

    // Move under gravity
    pos += vel * timeDeltaMs;

    vel.y += timeDeltaMs * 300;

    // The effect disappears after about 1 second
    return timer<1000;
}
void ExplosionFragment::Render()
{
    int size = g_TileSize * 2;
    size = size * (1000-timer) / 1000;
    DrawSpriteCentered(starImage, IW_FIXED_MUL(pos.x, g_TileSize), IW_FIXED_MUL(pos.y, g_TileSize), size);
}


//
// EffectManager class ////////////////////////////////////////////////////////////////////////
//

EffectManager::~EffectManager()
{
    Clear();
}

void EffectManager::Clear()
{
    for (uint32 i=0; i<effects.size(); i++)
        delete effects[i];
    effects.clear();
}

void EffectManager::Update(int timeDeltaMs)
{
    for (uint32 i=0; i<effects.size(); )
    {
        Effect* e = effects[i];
        if (!effects[i]->Update(timeDeltaMs))
        {
            // Note: this is quite inefficient if many objects are removed at the same time
            effects.erase(i);
            delete e;
        }
        else
            i++;
    }
}

void EffectManager::Render()
{
    Iw2DSetColour(0xff808080);
    Iw2DSetAlphaMode(IW_2D_ALPHA_ADD);
    for (uint32 i=0; i<effects.size(); i++)
        effects[i]->Render();
    Iw2DSetAlphaMode(IW_2D_ALPHA_NONE);
    Iw2DSetColour(0xffffffff);
}
