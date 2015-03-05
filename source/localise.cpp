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

#include "localise.h"

// Note:
//  strings should be UTF-8 encoded (since that's what GxFont uses)

const char * g_LocalisationEnglish[] = {
    "Score:",
    "Level:",
    "Game Over",
    "Orientation unsupported.\nPlease rotate your device.",
};

// Set language to english
const char ** g_Localisation = g_LocalisationEnglish;
