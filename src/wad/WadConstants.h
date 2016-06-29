/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
****/
#ifndef WAD_WADCONSTANTS_H
#define WAD_WADCONSTANTS_H

/**
*	File extension for wad files.
*/
#define WAD_FILE_EXT ".wad"

/**
*	Identifier for WAD2 (Quake) files.
*/
#define WAD2_ID "WAD2"

/**
*	Identifier for WAD3 (Half-Life) files.
*/
#define WAD3_ID "WAD3"

/**
*	Maximum size that a WAD lump name can be, including the null terminator.
*	TODO: this constant is used all over the place for different parts of the codebase. Define it in one place - Solokiller
*/
#define WAD_MAX_LUMP_NAME_SIZE 16

enum WadLumpType
{
	TYP_NONE			= 0,
	TYP_LABEL			= 1,

	/**
	*	64 + grab command number
	*/
	TYP_LUMPY			= 64,
};

/**
*	Possible values appended to TYP_LUMPY
*	@see WadLumpType
*	@see WadLumpType::TYP_LUMPY
*/
enum WadLumpyType
{
	TYP_LUMPY_PALETTE		= 0,
	TYP_LUMPY_COLORMAP,
	TYP_LUMPY_QPIC,
	TYP_LUMPY_MIPTEX,
	TYP_LUMPY_RAW,
	TYP_LUMPY_COLORMAP2,
	TYP_LUMPY_FONT
};

#define	MIPLEVELS	4

#endif //WAD_WADCONSTANTS_H