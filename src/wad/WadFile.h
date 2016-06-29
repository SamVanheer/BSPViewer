/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
****/
#ifndef WAD_WADFILE_H
#define WAD_WADFILE_H

#include "WadConstants.h"

/**
*	WAD file header.
*/
struct wadinfo_t
{
	/**
	*	Should be WAD2 or 2DAW
	*/
	char identification[ 4 ];

	/**
	*	Number of lumps in this WAD.
	*/
	int numlumps;

	/**
	*	Offset in this file where the first lump starts.
	*/
	int infotableofs;
};

/**
*	A single file entry in the WAD.
*/
struct lumpinfo_t
{
	/**
	*	Offset in this file where this lump's data starts.
	*/
	int filepos;

	/**
	*	Size of the lump's data. If this lump is compressed, this is the compressed size.
	*/
	int disksize;

	/**
	*	Size of the lump's data. This is the actual size of the lump.
	*/
	int size;

	/**
	*	Type of this lump.
	*	@see WadLumpType
	*	@see WadLumpyType
	*/
	char type;

	/**
	*	Indicates whether this lump is compressed or not.
	*	Not used by GoldSource, should always be false.
	*/
	bool compression;

	char pad1, pad2;

	/**
	*	Name of this lump.
	*	Must be null terminated.
	*/
	char name[ WAD_MAX_LUMP_NAME_SIZE ];
};

/**
*	Represents a single texture.
*/
struct miptex_t
{
	/**
	*	Name of this texture.
	*/
	char name[ WAD_MAX_LUMP_NAME_SIZE ];

	/**
	*	Width of this texture.
	*/
	unsigned width;

	/**
	*	Height of this texture.
	*/
	unsigned height;

	/**
	*	Mipmap offset for this texture.
	*	0 is the actual texture, 1-3 are mipmaps.
	*	Four mip maps stored.
	*	Pixels follow this structure.
	*	Image must be aligned at a 16 pixel boundary.
	*	Total number of pixels is ( ( width * height ) / 64 ) * 85 (total pixels / ( width * height ) / 64 ) = 85)
	*	Each mipmap is half as large as the previous.
	*	/ 1, / 2, / 4, / 8 => / 64
	*	If these offsets are all 0, the texture's data is not included in this file. (Used by BSPs for non-wadincluded textures)
	*/
	unsigned offsets[ MIPLEVELS ];
};

/**
*	Calculates the amount of bytes that the given miptex's pixels require.
*/
inline size_t GetMiptexPixelSize( const miptex_t& miptex )
{
	return ( miptex.width * miptex.height * 85 ) >> 6;
}

#endif //WAD_WADFILE_H