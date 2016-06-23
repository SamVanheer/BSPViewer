#ifndef BSP_BSPRENDERDEFS_H
#define BSP_BSPRENDERDEFS_H

#include <gl/glew.h>

#include "common/Const.h"
#include "utility/Mathlib.h"
#include "BSPConstants.h"
#include "BSPFile.h"

struct msurface_t;

/**
*	in memory representation
*	
*	!!! if this is changed, it must be changed in asm_draw.h too !!!
*/
struct mvertex_t
{
	Vector position;
};

#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2

/**
*	plane_t structure
*	!!! if this is changed, it must be changed in asm_i386.h too !!!
*/
struct mplane_t
{
	/**
	*	Plane normal.
	*/
	Vector normal;

	/**
	*	Dot product between the normal and the first vertex in the plane.
	*/
	float dist;

	/**
	*	for texture axis selection and fast side tests
	*	@see PlaneType
	*/
	byte type;

	/**
	*	Contains flags that tell whether the plane normal axes are positive or negative.
	*	signx + signy<<1 + signz<<2
	*/
	byte signbits;

	/**
	*	Padding to align the structure.
	*/
	byte pad[ 2 ];
};

/**
*	Represents a single texture.
*/
struct texture_t
{
	/**
	*	Texture name.
	*	TODO: use constant - Solokiller
	*/
	char name[ 16 ];

	/**
	*	Width.
	*/
	unsigned width;

	/**
	*	Height.
	*/
	unsigned height;

	/**
	*	glGenTextures value for this texture.
	*	TODO: glGenTextures uses GLuint. - Solokiller
	*/
	int gl_texturenum;

	/**
	*	for gl_texsort drawing
	*/
	msurface_t* texturechain;

	/**
	*	total tenths in sequence ( 0 = no)
	*/
	int anim_total;

	/**
	*	Time for this frame min <=time< max
	*/
	int anim_min;

	/**
	*	@copydoc anim_min
	*/
	int anim_max;

	/**
	*	in the animation sequence
	*/
	texture_t* anim_next;

	/**
	*	bmodels in frame 1 use these
	*/
	texture_t* alternate_anims;

	/**
	*	Offsets to mipmaps.
	*	four mip maps stored
	*/
	unsigned offsets[ MIPLEVELS ];
};

#define	SURF_PLANEBACK		2
#define	SURF_DRAWSKY		4
#define SURF_DRAWSPRITE		8
#define SURF_DRAWTURB		0x10
#define SURF_DRAWTILED		0x20
#define SURF_DRAWBACKGROUND	0x40
#define SURF_UNDERWATER		0x80

#define	BLOCK_WIDTH		128
#define	BLOCK_HEIGHT	128

/**
*	Represents a surface edge.
*	!!! if this is changed, it must be changed in asm_draw.h too !!!
*/
struct medge_t
{
	/**
	*	Vertex indices.
	*/
	unsigned short v[ 2 ];

	/**
	*	Not used by the GL renderer.
	*/
	unsigned int cachededgeoffset;
};

/**
*	Texture info.
*/
struct mtexinfo_t
{
	/**
	*	@copydoc texinfo_t::vecs
	*/
	float vecs[ 2 ][ 4 ];

	/**
	*	Not used by the GL renderer.
	*/
	float mipadjust;

	/**
	*	Texture used by the surfaces that use this texture info.
	*/
	texture_t* texture;

	/**
	*	Texture flags.
	*/
	int flags;
};

#define	VERTEXSIZE	7

/**
*	A single brush polygon.
*/
struct glpoly_t
{
	/**
	*	Next polygon for the owning brush model.
	*/
	glpoly_t* next;

	/**
	*	Used to build a list of lightmaps to render.
	*/
	glpoly_t* chain;

	/**
	*	Number of vertices in this polygon.
	*/
	int numverts;

	/**
	*	Polygon flags.
	*	for SURF_UNDERWATER
	*/
	int flags;

	/**
	*	Experimental. The OpenGL VBO ID.
	*/
	GLuint VBO;

	/**
	*	List of vertex commands.
	*	variable sized (xyz s1t1 s2t2)
	*	coordinate texture coordinates lightmap coordinates
	*	Actual size is numverts
	*/
	float verts[ 4 ][ VERTEXSIZE ];
};

struct msurface_t
{
	/**
	*	Frame number when this surface was last drawn.
	*	Should be drawn when node is crossed
	*/
	int visframe;

	/**
	*	Plane that this surface lies on.
	*/
	mplane_t* plane;

	/**
	*	Surface flags.
	*/
	int flags;

	/**
	*	First edge index.
	*	look up in model->surfedges[], negative numbers
	*	are backwards edges
	*/
	int firstedge;

	/**
	*	Number of edges.
	*	@see firstedge
	*/
	int numedges;

	/**
	*	Texture minimum bounds.
	*/
	short texturemins[ 2 ];

	/**
	*	Texture extents.
	*	texturemins + extents = maximum bounds.
	*/
	short extents[ 2 ];

	/**
	*	gl lightmap coordinates
	*/
	int light_s;

	/**
	*	@copydoc light_s
	*/
	int light_t;

	/**
	*	Multiple if warped
	*/
	glpoly_t* polys;

	/**
	*	List of surfaces to draw.
	*/
	msurface_t* texturechain;

	/**
	*	Texture info for this surface.
	*/
	mtexinfo_t	*texinfo;

	// lighting info

	/**
	*	Frame when lighting info was last applied to this surface.
	*/
	int dlightframe;

	/**
	*	Lighting bits.
	*	Each bit is a dynamic light. If set, it has been applied.
	*/
	int dlightbits;

	/**
	*	Lightmap texture number for this surface.
	*/
	int lightmaptexturenum;

	/**
	*	Light style indices.
	*/
	byte styles[ MAXLIGHTMAPS ];

	/**
	*	values currently used in lightmap
	*/
	int cached_light[ MAXLIGHTMAPS ];

	/**
	*	true if dynamic light in cache
	*/
	bool cached_dlight;

	/**
	*	Lighting data.
	*	[numstyles*surfsize]
	*/
	byte* samples;
};

struct mnode_t
{
	// common with leaf

	/**
	*	0, to differentiate from leafs
	*/
	int contents;

	/**
	*	node needs to be traversed if current
	*/
	int visframe;

	/**
	*	for bounding box culling
	*/
	Vector mins;

	/**
	*	@copydoc maxs
	*/
	Vector maxs;

	/**
	*	Parent node.
	*/
	mnode_t* parent;

	// node specific

	/**
	*	Plane that splits this node.
	*/
	mplane_t* plane;

	/**
	*	Child nodes.
	*/
	mnode_t* children[ 2 ];

	/**
	*	First surface in this node.
	*/
	unsigned short firstsurface;

	/**
	*	Number of surfaces.
	*/
	unsigned short numsurfaces;
};

/**
*	Fragment info?
*	TODO: move - Solokiller
*/
struct efrag_t
{
	struct mleaf_s		*leaf;
	struct efrag_s		*leafnext;
	struct entity_s		*entity;
	struct efrag_s		*entnext;
};

struct mleaf_t
{
	// common with node

	/**
	*	wil be a negative contents number
	*/
	int contents;

	/**
	*	node needs to be traversed if current
	*/
	int visframe;

	/**
	*	for bounding box culling
	*/
	Vector mins;

	/**
	*	@copydoc maxs
	*/
	Vector maxs;

	mnode_t* parent;

	// leaf specific

	/**
	*	Compressed vis data.
	*/
	byte* compressed_vis;

	/**
	*	Fragment info?
	*/
	efrag_t* efrags;

	/**
	*	Surfaces in this leaf.
	*/
	msurface_t** firstmarksurface;

	/**
	*	Number of surfaces in this leaf.
	*/
	int nummarksurfaces;

	/**
	*	BSP sequence number for leaf's contents
	*	Not used by the GL renderer.
	*/
	int key;

	/**
	*	Ambient sound level in this leaf.
	*	Not used in the GL renderer.
	*/
	byte ambient_sound_level[ NUM_AMBIENTS ];
};

/**
*	Hull data.
*	!!! if this is changed, it must be changed in asm_i386.h too !!!
*/
struct hull_t
{
	/**
	*	Clipnodes in this hull.
	*/
	dclipnode_t* clipnodes;

	/**
	*	Planes in this hull.
	*/
	mplane_t* planes;

	/**
	*	First clipnode used by this hull.
	*/
	int firstclipnode;

	/**
	*	Last clipnode used by this hull.
	*/
	int lastclipnode;

	/**
	*	Hull mins and maxs for entities collisions.
	*/
	Vector clip_mins;

	/**
	*	@copydoc clip_mins
	*/
	Vector clip_maxs;
};

struct bmodel_t
{
	char name[ MAX_QPATH ];

	int numframes;
	int flags;

	//
	// volume occupied by the model graphics
	//		
	Vector mins, maxs;
	float radius;

	//
	// solid volume for clipping 
	//
	bool clipbox;
	Vector clipmins, clipmaxs;

	//
	// brush model
	//
	int			firstmodelsurface, nummodelsurfaces;

	int			numsubmodels;
	dmodel_t	*submodels;

	int			numplanes;
	mplane_t	*planes;

	int			numleafs;		// number of visible leafs, not counting 0
	mleaf_t		*leafs;

	int			numvertexes;
	mvertex_t	*vertexes;

	int			numedges;
	medge_t		*edges;

	int			numnodes;
	mnode_t		*nodes;

	int			numtexinfo;
	mtexinfo_t	*texinfo;

	int			numsurfaces;
	msurface_t	*surfaces;

	int			numsurfedges;
	int			*surfedges;

	int			numclipnodes;
	dclipnode_t	*clipnodes;

	int			nummarksurfaces;
	msurface_t	**marksurfaces;

	hull_t		hulls[ MAX_MAP_HULLS ];

	int			numtextures;
	texture_t	**textures;

	byte		*visdata;
	byte		*lightdata;
	char		*entities;
};

#endif //BSP_BSPRENDERDEFS_H