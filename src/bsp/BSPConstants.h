/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
****/
#ifndef BSP_BSPCONSTANTS_H
#define BSP_BSPCONSTANTS_H

// upper design bounds

#define	MAX_MAP_HULLS		4

#define	MAX_MAP_MODELS		400
#define	MAX_MAP_BRUSHES		4096
#define	MAX_MAP_ENTITIES	1024
#define	MAX_MAP_ENTSTRING	(128*1024)

#define	MAX_MAP_PLANES		32767
#define	MAX_MAP_NODES		32767		// because negative shorts are contents
#define	MAX_MAP_CLIPNODES	32767		//
#define	MAX_MAP_LEAFS		8192
#define	MAX_MAP_VERTS		65535
#define	MAX_MAP_FACES		65535
#define	MAX_MAP_MARKSURFACES 65535
#define	MAX_MAP_TEXINFO		8192
#define	MAX_MAP_EDGES		256000
#define	MAX_MAP_SURFEDGES	512000
#define	MAX_MAP_TEXTURES	512
#define	MAX_MAP_MIPTEX		0x200000
#define	MAX_MAP_LIGHTING	0x200000
#define	MAX_MAP_VISIBILITY	0x200000

#define	MAX_MAP_PORTALS		65536

// key / value pair sizes

#define	MAX_KEY		32
#define	MAX_VALUE	1024

#define BSPVERSION	30
#define	TOOLVERSION	2 //TODO: what is this for? - Solokiller

#define	MAXLIGHTMAPS	4

#define	AMBIENT_WATER	0
#define	AMBIENT_SKY		1
#define	AMBIENT_SLIME	2
#define	AMBIENT_LAVA	3

#define	NUM_AMBIENTS			4		// automatic ambient sounds

#define	ANGLE_UP	-1
#define	ANGLE_DOWN	-2

enum BSPLump
{
	LUMP_FIRST			= 0,
	LUMP_ENTITIES		= LUMP_FIRST,
	LUMP_PLANES			= 1,
	LUMP_TEXTURES		= 2,
	LUMP_VERTEXES		= 3,
	LUMP_VISIBILITY		= 4,
	LUMP_NODES			= 5,
	LUMP_TEXINFO		= 6,
	LUMP_FACES			= 7,
	LUMP_LIGHTING		= 8,
	LUMP_CLIPNODES		= 9,
	LUMP_LEAFS			= 10,
	LUMP_MARKSURFACES	= 11,
	LUMP_EDGES			= 12,
	LUMP_SURFEDGES		= 13,
	LUMP_MODELS			= 14,

	LUMP_LAST			= LUMP_MODELS,
	
	HEADER_LUMPS		= 15
};

/**
*	0-2 are axial planes
*	3-5 are non-axial planes snapped to the nearest
*/
enum PlaneType
{
	PLANE_X		= 0,
	PLANE_Y		= 1,
	PLANE_Z		= 2,
 
	PLANE_ANYX	= 3,
	PLANE_ANYY	= 4,
	PLANE_ANYZ	= 5,
};

#define	CONTENTS_EMPTY		-1
#define	CONTENTS_SOLID		-2
#define	CONTENTS_WATER		-3
#define	CONTENTS_SLIME		-4
#define	CONTENTS_LAVA		-5
#define	CONTENTS_SKY		-6
#define	CONTENTS_ORIGIN		-7		// removed at csg time
#define	CONTENTS_CLIP		-8		// changed to contents_solid

#define	CONTENTS_CURRENT_0		-9
#define	CONTENTS_CURRENT_90		-10
#define	CONTENTS_CURRENT_180	-11
#define	CONTENTS_CURRENT_270	-12
#define	CONTENTS_CURRENT_UP		-13
#define	CONTENTS_CURRENT_DOWN	-14

#define CONTENTS_TRANSLUCENT	-15

/**
*	Texture flags.
*	@see texinfo_t
*/
enum TextureFlag
{
	/**
	*	sky or slime, no lightmap or 256 subdivision
	*/
	TEX_SPECIAL		= 1
};

#endif //BSP_BSPCONSTANTS_H