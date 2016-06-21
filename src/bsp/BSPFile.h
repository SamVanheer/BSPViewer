/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
****/
#ifndef BSP_BSPFILE_H
#define BSP_BSPFILE_H

#include "common/Const.h"
#include "BSPConstants.h"

#include "utility/Mathlib.h"

/**
*	Represents a chunk of data. Each chunk is one datatype.
*	@see BSPLump
*/
struct lump_t
{
	/**
	*	Byte offset in the file where this lump starts.
	*/
	int fileofs;

	/**
	*	Number of bytes in this lump.
	*/
	int filelen;
};

/**
*	Represents a brush model.
*/
struct dmodel_t
{
	/**
	*	Model bounds.
	*/
	Vector mins, maxs;

	/**
	*	Not used.
	*/
	Vector origin;

	/**
	*	Index for the node that represents the start of this BSP structure in each hull.
	*/
	int headnode[ MAX_MAP_HULLS ];

	/**
	*	Number of visible leafs.
	*	Not including the solid leaf 0
	*/
	int visleafs;

	/**
	*	First face that is part of the brush model.
	*/
	int firstface;

	/**
	*	Number of faces.
	*/
	int numfaces;
};

/**
*	Main header that represents the BSP data on disk.
*/
struct dheader_t
{
	/**
	*	BSP version.
	*/
	int version;

	/**
	*	Data lumps.
	*/
	lump_t lumps[ HEADER_LUMPS ];
};

/**
*	Lump header for the textures lump.
*/
struct dmiptexlump_t
{
	/**
	*	Number of textures in the BSP.
	*/
	int nummiptex;

	/**
	*	Byte offset relative to this header where the texture is located.
	*	This is actually an array of nummiptex size.
	*/
	int dataofs[ 4 ];
};

/**
*	Represents a single texture.
*/
struct miptex_t
{
	/**
	*	Name of this texture.
	*/
	char name[ 16 ];

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
	*/
	unsigned offsets[ MIPLEVELS ];
};

/**
*	Represents a single vertex.
*	Could probably replace this with a Vector array for convenience.
*/
struct dvertex_t
{
	Vector point;
};

/**
*	Represents a single plane in the BSP.
*/
struct dplane_t
{
	/**
	*	Plane normal.
	*/
	Vector normal;

	/**
	*	Dot product between normal and the first vertex.
	*/
	float dist;

	/**
	*	Plane type. Indicates whether this plane lies on an axial plane or not.
	*	PLANE_X - PLANE_ANYZ ?remove? trivial to regenerate
	*/
	int type;
};

/**
*	Represents a BSP node.
*	!!! if this is changed, it must be changed in asm_i386.h too !!!
*/
struct dnode_t
{
	/**
	*	Plane that splits this node.
	*/
	int planenum;

	/**
	*	Child nodes.
	*	Negative numbers are -(leafs+1), not nodes
	*/
	short children[ 2 ];

	/**
	*	Node mins.
	*/
	short mins[ 3 ];		// for sphere culling

	/**
	*	Node maxs.
	*/
	short maxs[ 3 ];

	/**
	*	First face that this node has.
	*/
	unsigned short firstface;

	/**
	*	Number of faces.
	*	counting both sides
	*/
	unsigned short numfaces;
};

/**
*	Represents a clipping node. This is part of a hull's physics mesh.
*/
struct dclipnode_t
{
	/**
	*	Plane that splits this node.
	*/
	int planenum;

	/**
	*	Child nodes.
	*	Negative numbers are contents.
	*/
	short children[ 2 ];
};

/**
*	Surface texture info.
*/
struct texinfo_t
{
	/**
	*	Contains horizontal and vertical texture info.
	*	xyz U/V axis, and offset
	*	
	*	[s/t][xyz offset]
	*/
	float vecs[ 2 ][ 4 ];

	/**
	*	Texture used by this surface.
	*/
	int miptex;

	/**
	*	Texture flags.
	*/
	int flags;
};

/*
*	Represents an edge.
*	note that edge 0 is never used, because negative edge nums are used for
*	counterclockwise use of the edge in a face
*/
struct dedge_t
{
	/**
	*	Vertex numbers
	*/
	unsigned short v[ 2 ];
};

/**
*	A single brush face.
*/
struct dface_t
{
	/**
	*	Plane that defines this face.
	*/
	short planenum;

	/**
	*	Which side of the plane this face is on.
	*/
	short side;

	/**
	*	First edge that defines this face.
	*	We must support > 64k edges.
	*/
	int firstedge;

	/**
	*	Number of edges. Equal to number of vertices.
	*/
	short numedges;

	/**
	*	Texture info for this face.
	*/
	short texinfo;

	// lighting info

	/**
	*	Light styles. These are indices into the global light styles.
	*/
	byte styles[ MAXLIGHTMAPS ];

	/**
	*	Lightmap data starting offset.
	*	Start of [numstyles*surfsize] samples
	*/
	int lightofs;
};

/**
*	leaf 0 is the generic CONTENTS_SOLID leaf, used for all solid areas
*	all other leafs need visibility info
*/
struct dleaf_t
{
	/**
	*	Leaf contents.
	*/
	int contents;

	/**
	*	Index into visibility map.
	*	-1 = no visibility info
	*/
	int visofs;

	/**
	*	For frustum culling
	*/
	short mins[ 3 ];
	short maxs[ 3 ];

	/**
	*	First surface that is in this leaf.
	*/
	unsigned short firstmarksurface;

	/**
	*	Number of surfaces that are in this leaf.
	*/
	unsigned short nummarksurfaces;

	/**
	*	Obsolete. Used in Quake for ambient sounds. Half-Life doesn't play ambient sounds automatically.
	*/
	byte ambient_level[ NUM_AMBIENTS ];
};

//TODO: these should be classes - Solokiller
struct epair_t
{
	epair_t*	next;
	char*		key;
	char*		value;
};

struct entity_t
{
	Vector		origin;
	int			firstbrush;
	int			numbrushes;
	epair_t*	epairs;
};

template<typename TYPE, const size_t SIZE>
struct BSPData
{
	typedef TYPE Type_t;
	static const size_t MAX_SIZE = SIZE;

	int checksum;
	int count;
	TYPE data[ SIZE ];
};

typedef BSPData<dmodel_t, MAX_MAP_MODELS> models_data;
typedef BSPData<byte, MAX_MAP_VISIBILITY> visdata_data;
typedef BSPData<byte, MAX_MAP_LIGHTING> lightdata_data;
typedef BSPData<byte, MAX_MAP_MIPTEX> texdata_data;
typedef BSPData<char, MAX_MAP_ENTSTRING> entdata_data;
typedef BSPData<dleaf_t, MAX_MAP_LEAFS> leafs_data;
typedef BSPData<dplane_t, MAX_MAP_PLANES> planes_data;
typedef BSPData<dvertex_t, MAX_MAP_VERTS> vertexes_data;
typedef BSPData<dnode_t, MAX_MAP_NODES> nodes_data;
typedef BSPData<texinfo_t, MAX_MAP_TEXINFO> texinfo_data;
typedef BSPData<dface_t, MAX_MAP_FACES> faces_data;
typedef BSPData<dclipnode_t, MAX_MAP_CLIPNODES> clipnodes_data;
typedef BSPData<dedge_t, MAX_MAP_EDGES> edges_data;
typedef BSPData<unsigned short, MAX_MAP_MARKSURFACES> marksurfaces_data;
typedef BSPData<int, MAX_MAP_SURFEDGES> surfedges_data;

/**
*	A single BSP file.
*	TODO: currently stores all of its data as huge arrays. Maybe consider an alternate approach - Solokiller
*/
class CBSPFile
{
public:
	models_data models;
	visdata_data visdata;
	lightdata_data lightdata;
	// (dmiptexlump_t)
	texdata_data texdata;
	entdata_data entdata;
	leafs_data leafs;
	planes_data planes;
	vertexes_data vertexes;
	nodes_data nodes;
	texinfo_data texinfo;
	faces_data faces;
	clipnodes_data clipnodes;
	edges_data edges;
	marksurfaces_data marksurfaces;
	surfedges_data surfedges;
};

#endif //BSP_BSPFILE_H