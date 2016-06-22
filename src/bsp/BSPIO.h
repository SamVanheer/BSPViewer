/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
****/
#ifndef BSP_BSPIO_H
#define BSP_BSPIO_H

#include <memory>

#include "BSPConstants.h"
#include "BSPFile.h"

std::unique_ptr<dheader_t> LoadBSPFile( const char* const pszFileName );

bool LoadBSPFile( const char* const pszFileName, CBSPFile& file );

void SwapBSPFile( const bool todisk, CBSPFile& file );

#endif //BSP_BSPIO_H