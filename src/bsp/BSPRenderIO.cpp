#include <cmath>
#include <cstdio>
#include <memory>

#include <glm/gtc/type_ptr.hpp>

#include "utility/ByteSwap.h"
#include "utility/Tokenization.h"

#include "gl/CShaderManager.h"
#include "gl/CBaseShader.h"

#include "wad/CWadManager.h"
#include "gl/CTextureManager.h"

#include "BSPRenderIO.h"

namespace BSP
{
/*
=================
Mod_LoadVertexes
=================
*/
bool Mod_LoadVertexes( bmodel_t* pModel, dheader_t* pHeader, lump_t* l )
{
	byte* mod_base = reinterpret_cast<byte*>( pHeader );

	dvertex_t* in = ( dvertex_t* ) ( mod_base + l->fileofs );

	if( l->filelen % sizeof( *in ) )
	{
		printf( "MOD_LoadBmodel: funny lump size in %s\n", pModel->name );
		return false;
	}

	const size_t count = l->filelen / sizeof( *in );

	mvertex_t* out = new mvertex_t[ count ];

	pModel->vertexes = out;
	pModel->numvertexes = count;

	for( size_t i = 0; i<count; i++, in++, out++ )
	{
		out->position[ 0 ] = LittleValue( in->point[ 0 ] );
		out->position[ 1 ] = LittleValue( in->point[ 1 ] );
		out->position[ 2 ] = LittleValue( in->point[ 2 ] );
	}

	return true;
}

/*
=================
Mod_LoadEdges
=================
*/
bool Mod_LoadEdges( bmodel_t* pModel, dheader_t* pHeader, lump_t* l )
{
	byte* mod_base = reinterpret_cast<byte*>( pHeader );

	dedge_t* in = ( dedge_t* ) ( mod_base + l->fileofs );

	if( l->filelen % sizeof( *in ) )
	{
		printf( "MOD_LoadBmodel: funny lump size in %s\n", pModel->name );
		return false;
	}

	const size_t count = l->filelen / sizeof( *in );
	medge_t* out = new medge_t[ count ];

	pModel->edges = out;
	pModel->numedges = count;

	for( size_t i = 0; i<count; i++, in++, out++ )
	{
		out->v[ 0 ] = ( unsigned short ) LittleValue( in->v[ 0 ] );
		out->v[ 1 ] = ( unsigned short ) LittleValue( in->v[ 1 ] );
	}

	return true;
}

/*
=================
Mod_LoadSurfedges
=================
*/
bool Mod_LoadSurfedges( bmodel_t* pModel, dheader_t* pHeader, lump_t* l )
{
	byte* mod_base = reinterpret_cast<byte*>( pHeader );

	int* in = ( int* ) ( mod_base + l->fileofs );

	if( l->filelen % sizeof( *in ) )
	{
		printf( "MOD_LoadBmodel: funny lump size in %s\n", pModel->name );
		return false;
	}

	const size_t count = l->filelen / sizeof( *in );

	int* out = new int[ count ];

	pModel->surfedges = out;
	pModel->numsurfedges = count;

	for( size_t i = 0; i < count; ++i )
		out[ i ] = LittleValue( in[ i ] );

	return true;
}

/*
=================
Mod_LoadTextures
=================
*/
bool Mod_LoadTextures( bmodel_t* pModel, dheader_t* pHeader, lump_t* l )
{
	int		j, pixels;
	miptex_t	*mt;

	byte* mod_base = reinterpret_cast<byte*>( pHeader );

	//No textures to load.
	if( !l->filelen )
	{
		pModel->textures = nullptr;
		return true;
	}

	dmiptexlump_t* m = ( dmiptexlump_t* ) ( mod_base + l->fileofs );

	m->nummiptex = LittleValue( m->nummiptex );

	pModel->numtextures = m->nummiptex;
	pModel->textures = new texture_t*[ m->nummiptex ];

	memset( pModel->textures, 0, sizeof( texture_t* ) * m->nummiptex );

	if( !g_TextureManager.Initialize( m->nummiptex ) )
		return false;

	for( int i = 0; i<m->nummiptex; i++ )
	{
		m->dataofs[ i ] = LittleValue( m->dataofs[ i ] );
		if( m->dataofs[ i ] == -1 )
			continue;
		mt = ( miptex_t * ) ( ( byte * ) m + m->dataofs[ i ] );
		mt->width = LittleValue( mt->width );
		mt->height = LittleValue( mt->height );
		for( j = 0; j<MIPLEVELS; j++ )
			mt->offsets[ j ] = LittleValue( mt->offsets[ j ] );

		if( ( mt->width & 15 ) || ( mt->height & 15 ) )
		{
			printf( "Texture %s is not 16 aligned\n", mt->name );
			return false;
		}
		pixels = mt->width*mt->height / 64 * 85;

		//Load the texture. Internal or external, doesn't matter.
		//TODO: the textures array is obsolete now. - Solokiller
		pModel->textures[ i ] = g_TextureManager.LoadTexture( mt->name, mt->offsets[ 0 ] > 0 ? mt : nullptr );

		/*
		TODO: fix this - Solokiller
		if( !strncmp( mt->name, "sky", 3 ) )
			R_InitSky( tx );
		else
		{
			texture_mode = GL_LINEAR_MIPMAP_NEAREST; //_LINEAR;
			tx->gl_texturenum = GL_LoadTexture( mt->name, tx->width, tx->height, ( byte * ) ( tx + 1 ), true, false );
			texture_mode = GL_LINEAR;
		}
		*/
	}

	//Wads aren't needed anymore now.
	g_WadManager.Clear();

	if( !g_TextureManager.SetupAnimatingTextures() )
	{
		printf( "Couldn't set up animating textures\n" );
		return false;
	}

	return true;
}

/*
=================
Mod_LoadLighting
=================
*/
bool Mod_LoadLighting( bmodel_t* pModel, dheader_t* pHeader, lump_t* l )
{
	//Nothing to load.
	if( !l->filelen )
	{
		pModel->lightdata = nullptr;
		return true;
	}

	pModel->lightdata = new byte[ l->filelen ];
	memcpy( pModel->lightdata, ( reinterpret_cast<byte*>( pHeader ) ) + l->fileofs, l->filelen );

	return true;
}

/*
=================
Mod_LoadPlanes
=================
*/
bool Mod_LoadPlanes( bmodel_t* pModel, dheader_t* pHeader, lump_t* l )
{
	byte* mod_base = reinterpret_cast<byte*>( pHeader );

	dplane_t* in = ( dplane_t* ) ( mod_base + l->fileofs );

	if( l->filelen % sizeof( *in ) )
	{
		printf( "MOD_LoadBmodel: funny lump size in %s\n", pModel->name );
		return false;
	}

	const size_t count = l->filelen / sizeof( *in );

	//TODO: Why * 2? - Solokiller
	mplane_t* out = new mplane_t[ count * 2 ];

	pModel->planes = out;
	pModel->numplanes = count;

	int bits;

	for( size_t i = 0; i<count; i++, in++, out++ )
	{
		bits = 0;
		for( size_t j = 0; j<3; j++ )
		{
			out->normal[ j ] = LittleValue( in->normal[ j ] );
			if( out->normal[ j ] < 0 )
				bits |= 1 << j;
		}

		out->dist = LittleValue( in->dist );
		out->type = LittleValue( in->type );
		out->signbits = bits;
	}

	return true;
}

//TODO: should be checkboard - Solokiller
texture_t* r_notexture_mip = nullptr;

/*
=================
Mod_LoadTexinfo
=================
*/
bool Mod_LoadTexinfo( bmodel_t* pModel, dheader_t* pHeader, lump_t* l )
{
	int		miptex;
	float	len1, len2;

	byte* mod_base = reinterpret_cast<byte*>( pHeader );

	texinfo_t* in = ( texinfo_t* ) ( mod_base + l->fileofs );

	if( l->filelen % sizeof( *in ) )
	{
		printf( "MOD_LoadBmodel: funny lump size in %s\n", pModel->name );
		return false;
	}

	const size_t count = l->filelen / sizeof( *in );

	mtexinfo_t* out = new mtexinfo_t[ count ];

	pModel->texinfo = out;
	pModel->numtexinfo = count;

	for( size_t i = 0; i<count; i++, in++, out++ )
	{
		for( size_t j = 0; j<8; j++ )
			out->vecs[ 0 ][ j ] = LittleValue( in->vecs[ 0 ][ j ] );
		len1 = glm::length( *reinterpret_cast<Vector*>( &out->vecs[ 0 ] ) );
		len2 = glm::length( *reinterpret_cast<Vector*>( &out->vecs[ 1 ] ) );
		len1 = ( len1 + len2 ) / 2;
		//TODO: is mipadjust used? - Solokliler
		if( len1 < 0.32 )
			out->mipadjust = 4;
		else if( len1 < 0.49 )
			out->mipadjust = 3;
		else if( len1 < 0.99 )
			out->mipadjust = 2;
		else
			out->mipadjust = 1;
#if 0
		if( len1 + len2 < 0.001 )
			out->mipadjust = 1;		// don't crash
		else
			out->mipadjust = 1 / floor( ( len1 + len2 ) / 2 + 0.1 );
#endif

		miptex = LittleValue( in->miptex );
		out->flags = LittleValue( in->flags );

		if( !pModel->textures )
		{
			out->texture = r_notexture_mip;	// checkerboard texture
			out->flags = 0;
		}
		else
		{
			if( miptex >= pModel->numtextures )
			{
				printf( "miptex >= loadmodel->numtextures\n" );
				return false;
			}

			out->texture = pModel->textures[ miptex ];
			if( !out->texture )
			{
				out->texture = r_notexture_mip; // texture not found
				out->flags = 0;
			}
		}
	}

	return true;
}

/*
================
CalcSurfaceExtents

Fills in s->texturemins[] and s->extents[]
================
*/
bool CalcSurfaceExtents( bmodel_t* pModel, msurface_t *s )
{
	float	mins[ 2 ], maxs[ 2 ], val;
	int		i, j, e;
	mvertex_t	*v;
	mtexinfo_t	*tex;
	int		bmins[ 2 ], bmaxs[ 2 ];

	mins[ 0 ] = mins[ 1 ] = 999999;
	maxs[ 0 ] = maxs[ 1 ] = -99999;

	tex = s->texinfo;

	for( i = 0; i<s->numedges; i++ )
	{
		e = pModel->surfedges[ s->firstedge + i ];
		if( e >= 0 )
			v = &pModel->vertexes[ pModel->edges[ e ].v[ 0 ] ];
		else
			v = &pModel->vertexes[ pModel->edges[ -e ].v[ 1 ] ];

		for( j = 0; j<2; j++ )
		{
			val = v->position[ 0 ] * tex->vecs[ j ][ 0 ] +
				v->position[ 1 ] * tex->vecs[ j ][ 1 ] +
				v->position[ 2 ] * tex->vecs[ j ][ 2 ] +
				tex->vecs[ j ][ 3 ];
			if( val < mins[ j ] )
				mins[ j ] = val;
			if( val > maxs[ j ] )
				maxs[ j ] = val;
		}
	}

	for( i = 0; i<2; i++ )
	{
		bmins[ i ] = static_cast<int>( floor( mins[ i ] / 16 ) );
		bmaxs[ i ] = static_cast<int>( ceil( maxs[ i ] / 16 ) );

		s->texturemins[ i ] = bmins[ i ] * 16;
		s->extents[ i ] = ( bmaxs[ i ] - bmins[ i ] ) * 16;
		if( !( tex->flags & TEX_SPECIAL ) && s->extents[ i ] > 512 /* 256 */ )
		{
			printf( "Bad surface extents\n" );
			return false;
		}
	}

	return true;
}

void BoundPoly( int numverts, Vector* verts, Vector& mins, Vector& maxs )
{
	//TODO: change to INT_MIN & INT_MAX to prevent large maps from breaking - Solokiller
	mins[ 0 ] = mins[ 1 ] = mins[ 2 ] = 9999;
	maxs[ 0 ] = maxs[ 1 ] = maxs[ 2 ] = -9999;

	float* v = glm::value_ptr( *verts );

	for( int i = 0; i<numverts; i++ )
		for( int j = 0; j<3; j++, v++ )
		{
			if( *v < mins[ j ] )
				mins[ j ] = *v;
			if( *v > maxs[ j ] )
				maxs[ j ] = *v;
		}
}

//static size_t g_uiPolyCount = 0;

void CreatePoly( glpoly_t* pPoly )
{
	//glGenVertexArrays( 1, &pPoly->VAO );
	//glBindVertexArray( pPoly->VAO );

	glGenBuffers( 1, &pPoly->VBO );
	glBindBuffer( GL_ARRAY_BUFFER, pPoly->VBO );

	glBufferData( GL_ARRAY_BUFFER, pPoly->numverts * VERTEXSIZE * sizeof( GLfloat ), pPoly->verts, GL_STATIC_DRAW );

	g_ShaderManager.GetShader( "LightMappedGeneric" )->SetupVertexAttribs();

	/*
	++g_uiPolyCount;

	printf( "Created poly %u%s\n", g_uiPolyCount, pPoly->next ? " (duplicate)" : "" );
	*/
}

bool SubdividePolygon( msurface_t* pSurface, int numverts, Vector* verts )
{
	int		i, j, k;
	Vector	mins, maxs;
	float	m;
	Vector	*v;
	Vector	front[ 64 ], back[ 64 ];
	int		f, b;
	float	dist[ 64 ];
	float	frac;
	glpoly_t	*poly;
	float	s, t;

	if( numverts > 60 )
	{
		printf( "numverts = %i", numverts );
		return false;
	}

	BoundPoly( numverts, verts, mins, maxs );

	for( i = 0; i<3; i++ )
	{
		m = ( mins[ i ] + maxs[ i ] ) * 0.5f;
		m = /*gl_subdivide_size.value*/ 128 * floor( m / 28 + 0.5f );
		if( maxs[ i ] - m < 8 )
			continue;
		if( m - mins[ i ] < 8 )
			continue;

		// cut it
		v = verts + i;
		for( j = 0; j<numverts; j++, ++v )
			dist[ j ] = ( *v ).x - m;

		// wrap cases
		dist[ j ] = dist[ 0 ];
		v -= i;
		*v = *verts;

		f = b = 0;
		v = verts;
		for( j = 0; j<numverts; j++, v += 3 )
		{
			if( dist[ j ] >= 0 )
			{
				front[ f ] = *v;
				f++;
			}
			if( dist[ j ] <= 0 )
			{
				back[ b ] = *v;
				b++;
			}
			if( dist[ j ] == 0 || dist[ j + 1 ] == 0 )
				continue;
			if( ( dist[ j ] > 0 ) != ( dist[ j + 1 ] > 0 ) )
			{
				// clip point
				frac = dist[ j ] / ( dist[ j ] - dist[ j + 1 ] );
				for( k = 0; k<3; k++ )
					front[ f ][ k ] = back[ b ][ k ] = ( *v )[ k ] + frac*( ( *v )[ 3 + k ] - ( *v )[ k ] );
				f++;
				b++;
			}
		}

		SubdividePolygon( pSurface, f, front );
		SubdividePolygon( pSurface, b, back );
		return true;
	}

	const size_t uiSize = sizeof( glpoly_t ) + ( numverts - 4 ) * VERTEXSIZE * sizeof( float );

	poly = reinterpret_cast<glpoly_t*>( new byte[ uiSize ] );

	memset( poly, 0, uiSize );

	poly->next = pSurface->polys;
	pSurface->polys = poly;
	poly->numverts = numverts;
	for( i = 0; i<numverts; i++, ++verts )
	{
		*reinterpret_cast<Vector*>( &poly->verts[ i ] ) = *verts;
		s = glm::dot( *verts, *reinterpret_cast<Vector*>( &pSurface->texinfo->vecs[ 0 ] ) );
		t = glm::dot( *verts, *reinterpret_cast<Vector*>( &pSurface->texinfo->vecs[ 1 ] ) );
		poly->verts[ i ][ 3 ] = s;
		poly->verts[ i ][ 4 ] = t;
	}

	CreatePoly( poly );

	return true;
}

/*
================
GL_SubdivideSurface

Breaks a polygon up along axial 64 unit
boundaries so that turbulent and sky warps
can be done reasonably.
================
*/
bool GL_SubdivideSurface( bmodel_t* pModel, msurface_t *fa )
{
	Vector		verts[ 64 ];
	int			lindex;
	Vector* pVec;

	//
	// convert edges back to a normal polygon
	//
	int numverts = 0;
	for( int i = 0; i<fa->numedges; i++ )
	{
		lindex = pModel->surfedges[ fa->firstedge + i ];

		if( lindex > 0 )
			pVec = &pModel->vertexes[ pModel->edges[ lindex ].v[ 0 ] ].position;
		else
			pVec = &pModel->vertexes[ pModel->edges[ -lindex ].v[ 1 ] ].position;
		verts[ numverts ] = *pVec;
		numverts++;
	}

	return SubdividePolygon( fa, numverts, verts );
}

/*
=================
Mod_LoadFaces
=================
*/
bool Mod_LoadFaces( bmodel_t* pModel, dheader_t* pHeader, lump_t* l )
{
	int			planenum, side;

	byte* mod_base = reinterpret_cast<byte*>( pHeader );

	dface_t* in = ( dface_t* ) ( mod_base + l->fileofs );

	if( l->filelen % sizeof( *in ) )
	{
		printf( "MOD_LoadBmodel: funny lump size in %s\n", pModel->name );
		return false;
	}

	const size_t count = l->filelen / sizeof( *in );
	msurface_t* out = new msurface_t[ count ];

	memset( out, 0, sizeof( msurface_t ) * count );

	pModel->surfaces = out;
	pModel->numsurfaces = count;

	int i;

	for( size_t surfnum = 0; surfnum<count; surfnum++, in++, out++ )
	{
		out->firstedge = LittleValue( in->firstedge );
		out->numedges = LittleValue( in->numedges );
		out->flags = 0;

		planenum = LittleValue( in->planenum );
		side = LittleValue( in->side );
		if( side )
			out->flags |= SURF_PLANEBACK;

		out->plane = pModel->planes + planenum;

		out->texinfo = pModel->texinfo + LittleValue( in->texinfo );

		if( !CalcSurfaceExtents( pModel, out ) )
		{
			return false;
		}

		// lighting info

		for( i = 0; i<MAXLIGHTMAPS; i++ )
			out->styles[ i ] = in->styles[ i ];
		i = LittleValue( in->lightofs );
		if( i == -1 )
			out->samples = NULL;
		else
			out->samples = pModel->lightdata + i;

		// set the drawing flags flag

		/*
		if( !strncmp( out->texinfo->texture->name, "sky", 3 ) )	// sky
		{
			out->flags |= ( SURF_DRAWSKY | SURF_DRAWTILED );
#ifndef QUAKE2
			// cut up polygon for warps
			if( !GL_SubdivideSurface( pModel, out ) )
				return false;
#endif
			continue;
		}

		if( !strncmp( out->texinfo->texture->name, "*", 1 ) )		// turbulent
		{
			out->flags |= ( SURF_DRAWTURB | SURF_DRAWTILED );
			for( i = 0; i<2; i++ )
			{
				//TODO: if these are used in GoldSource, tune them so they can fit in large maps - Solokiller
				out->extents[ i ] = 16384;
				out->texturemins[ i ] = -8192;
			}
			// cut up polygon for warps
			if( !GL_SubdivideSurface( pModel, out ) )
				return false;
			continue;
		}
		*/

	}

	return true;
}

/*
=================
Mod_LoadMarksurfaces
=================
*/
bool Mod_LoadMarksurfaces( bmodel_t* pModel, dheader_t* pHeader, lump_t* l )
{
	byte* mod_base = reinterpret_cast<byte*>( pHeader );

	short* in = ( short* ) ( mod_base + l->fileofs );

	if( l->filelen % sizeof( *in ) )
	{
		printf( "MOD_LoadBmodel: funny lump size in %s\n", pModel->name );
		return false;
	}

	const size_t count = l->filelen / sizeof( *in );
	msurface_t** out = new msurface_t*[ count ];

	pModel->marksurfaces = out;
	pModel->nummarksurfaces = count;

	int j;

	for( size_t i = 0; i<count; i++ )
	{
		j = LittleValue( in[ i ] );

		if( j >= pModel->numsurfaces )
		{
			printf( "Mod_ParseMarksurfaces: bad surface number\n" );
			return false;
		}

		out[ i ] = pModel->surfaces + j;
	}

	return true;
}

/*
=================
Mod_LoadVisibility
=================
*/
bool Mod_LoadVisibility( bmodel_t* pModel, dheader_t* pHeader, lump_t* l )
{
	//Nothing to load.
	if( !l->filelen )
	{
		pModel->visdata = nullptr;
		return false;
	}

	pModel->visdata = new byte[ l->filelen ];
	memcpy( pModel->visdata, reinterpret_cast<byte*>( pHeader ) + l->fileofs, l->filelen );

	return true;
}

/*
=================
Mod_LoadLeafs
=================
*/
bool Mod_LoadLeafs( bmodel_t* pModel, dheader_t* pHeader, lump_t* l )
{
	byte* mod_base = reinterpret_cast<byte*>( pHeader );

	dleaf_t* in = ( dleaf_t* ) ( mod_base + l->fileofs );

	if( l->filelen % sizeof( *in ) )
	{
		printf( "MOD_LoadBmodel: funny lump size in %s\n", pModel->name );
		return false;
	}

	const size_t count = l->filelen / sizeof( *in );
	mleaf_t* out = new mleaf_t[ count ];

	pModel->leafs = out;
	pModel->numleafs = count;

	int p;

	for( size_t i = 0; i<count; i++, in++, out++ )
	{
		for( size_t j = 0; j<3; j++ )
		{
			out->mins[ j ] = LittleValue( in->mins[ j ] );
			out->maxs[ j ] = LittleValue( in->maxs[ j ] );
		}

		p = LittleValue( in->contents );
		out->contents = p;

		out->firstmarksurface = pModel->marksurfaces +
			LittleValue( in->firstmarksurface );
		out->nummarksurfaces = LittleValue( in->nummarksurfaces );

		p = LittleValue( in->visofs );
		if( p == -1 )
			out->compressed_vis = NULL;
		else
			out->compressed_vis = pModel->visdata + p;
		out->efrags = NULL;

		for( size_t j = 0; j<4; j++ )
			out->ambient_sound_level[ j ] = in->ambient_level[ j ];

		// gl underwater warp
		if( out->contents != CONTENTS_EMPTY )
		{
			for( int j = 0; j<out->nummarksurfaces; j++ )
				out->firstmarksurface[ j ]->flags |= SURF_UNDERWATER;
		}
	}

	return true;
}

/*
=================
Mod_SetParent
=================
*/
void Mod_SetParent( mnode_t *node, mnode_t *parent )
{
	node->parent = parent;
	if( node->contents < 0 )
		return;
	Mod_SetParent( node->children[ 0 ], node );
	Mod_SetParent( node->children[ 1 ], node );
}

/*
=================
Mod_LoadNodes
=================
*/
bool Mod_LoadNodes( bmodel_t* pModel, dheader_t* pHeader, lump_t* l )
{
	byte* mod_base = reinterpret_cast<byte*>( pHeader );

	dnode_t* in = ( dnode_t* ) ( mod_base + l->fileofs );

	if( l->filelen % sizeof( *in ) )
	{
		printf( "MOD_LoadBmodel: funny lump size in %s\n", pModel->name );
		return false;
	}

	const size_t count = l->filelen / sizeof( *in );
	mnode_t* out = new mnode_t[ count ];

	pModel->nodes = out;
	pModel->numnodes = count;

	int p;

	for( size_t i = 0; i<count; i++, in++, out++ )
	{
		for( size_t j = 0; j<3; j++ )
		{
			out->mins[ j ] = LittleValue( in->mins[ j ] );
			out->maxs[ j ] = LittleValue( in->maxs[ j ] );
		}

		p = LittleValue( in->planenum );
		out->plane = pModel->planes + p;

		out->firstsurface = LittleValue( in->firstface );
		out->numsurfaces = LittleValue( in->numfaces );

		for( size_t j = 0; j<2; j++ )
		{
			p = LittleValue( in->children[ j ] );
			if( p >= 0 )
				out->children[ j ] = pModel->nodes + p;
			else
				out->children[ j ] = ( mnode_t * ) ( pModel->leafs + ( -1 - p ) );
		}
	}

	Mod_SetParent( pModel->nodes, NULL );	// sets nodes and leafs

	return true;
}

/*
=================
Mod_LoadClipnodes
=================
*/
bool Mod_LoadClipnodes( bmodel_t* pModel, dheader_t* pHeader, lump_t* l )
{
	byte* mod_base = reinterpret_cast<byte*>( pHeader );

	dclipnode_t* in = ( dclipnode_t* ) ( mod_base + l->fileofs );

	if( l->filelen % sizeof( *in ) )
	{
		printf( "MOD_LoadBmodel: funny lump size in %s\n", pModel->name );
		return false;
	}

	const size_t count = l->filelen / sizeof( *in );
	dclipnode_t* out = new dclipnode_t[ count ];

	pModel->clipnodes = out;
	pModel->numclipnodes = count;

	hull_t* hull;

	hull = &pModel->hulls[ 1 ];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count - 1;
	hull->planes = pModel->planes;
	hull->clip_mins[ 0 ] = -16;
	hull->clip_mins[ 1 ] = -16;
	hull->clip_mins[ 2 ] = -24;
	hull->clip_maxs[ 0 ] = 16;
	hull->clip_maxs[ 1 ] = 16;
	hull->clip_maxs[ 2 ] = 32;

	hull = &pModel->hulls[ 2 ];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count - 1;
	hull->planes = pModel->planes;
	hull->clip_mins[ 0 ] = -32;
	hull->clip_mins[ 1 ] = -32;
	hull->clip_mins[ 2 ] = -24;
	hull->clip_maxs[ 0 ] = 32;
	hull->clip_maxs[ 1 ] = 32;
	hull->clip_maxs[ 2 ] = 64;

	for( size_t i = 0; i<count; i++, out++, in++ )
	{
		out->planenum = LittleValue( in->planenum );
		out->children[ 0 ] = LittleValue( in->children[ 0 ] );
		out->children[ 1 ] = LittleValue( in->children[ 1 ] );
	}

	return true;
}

/*
=================
Mod_LoadEntities
=================
*/
bool Mod_LoadEntities( bmodel_t* pModel, dheader_t* pHeader, lump_t* l )
{
	//Nothing to load.
	if( !l->filelen )
	{
		pModel->entities = nullptr;
		return true;
	}

	pModel->entities = new char[ l->filelen ];
	memcpy( pModel->entities, reinterpret_cast<byte*>( pHeader ) + l->fileofs, l->filelen );

	return true;
}

/*
=================
Mod_LoadSubmodels
=================
*/
bool Mod_LoadSubmodels( bmodel_t* pModel, dheader_t* pHeader, lump_t* l )
{
	byte* mod_base = reinterpret_cast<byte*>( pHeader );

	dmodel_t* in = ( dmodel_t* ) ( mod_base + l->fileofs );

	if( l->filelen % sizeof( *in ) )
	{
		printf( "MOD_LoadBmodel: funny lump size in %s", pModel->name );
		return false;
	}

	const size_t count = l->filelen / sizeof( *in );
	dmodel_t* out = new dmodel_t[ count ];

	pModel->submodels = out;
	pModel->numsubmodels = count;

	for( size_t i = 0; i<count; i++, in++, out++ )
	{
		for( size_t j = 0; j<3; j++ )
		{	// spread the mins / maxs by a pixel
			out->mins[ j ] = LittleValue( in->mins[ j ] ) - 1;
			out->maxs[ j ] = LittleValue( in->maxs[ j ] ) + 1;
			out->origin[ j ] = LittleValue( in->origin[ j ] );
		}
		for( size_t j = 0; j<MAX_MAP_HULLS; j++ )
			out->headnode[ j ] = LittleValue( in->headnode[ j ] );
		out->visleafs = LittleValue( in->visleafs );
		out->firstface = LittleValue( in->firstface );
		out->numfaces = LittleValue( in->numfaces );
	}

	return true;
}

/*
=================
Mod_MakeHull0

Deplicate the drawing hull structure as a clipping hull
=================
*/
void Mod_MakeHull0( bmodel_t* pModel )
{
	hull_t* hull = &pModel->hulls[ 0 ];

	mnode_t* in = pModel->nodes;
	const size_t count = pModel->numnodes;
	dclipnode_t* out = new dclipnode_t[ count ];

	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count - 1;
	hull->planes = pModel->planes;

	mnode_t* child;

	for( size_t i = 0; i<count; i++, out++, in++ )
	{
		out->planenum = in->plane - pModel->planes;
		for( size_t j = 0; j<2; j++ )
		{
			child = in->children[ j ];
			if( child->contents < 0 )
				out->children[ j ] = child->contents;
			else
				out->children[ j ] = child - pModel->nodes;
		}
	}
}

/*
=================
RadiusFromBounds
=================
*/
float RadiusFromBounds( const Vector& mins, const Vector& maxs )
{
	Vector	corner;

	for( size_t i = 0; i < 3; ++i )
	{
		corner[ i ] = fabs( mins[ i ] ) > fabs( maxs[ i ] ) ? fabs( mins[ i ] ) : fabs( maxs[ i ] );
	}

	return glm::length( corner );
}

bmodel_t mod_known[ MAX_MOD_KNOWN ];
int mod_numknown = 0;

/*
==================
Mod_FindName

==================
*/
bmodel_t* Mod_FindName( char* name )
{
	int		i;
	bmodel_t* mod;

	if( !name[ 0 ] )
	{
		printf( "Mod_ForName: NULL name" );
		return nullptr;
	}

	//
	// search the currently loaded models
	//
	for( i = 0, mod = mod_known; i<mod_numknown; i++, mod++ )
		if( !strcmp( mod->name, name ) )
			break;

	if( i == mod_numknown )
	{
		if( mod_numknown == MAX_MOD_KNOWN )
		{
			printf( "mod_numknown == MAX_MOD_KNOWN" );
			return nullptr;
		}
		strcpy( mod->name, name );
		mod_numknown++;
	}

	return mod;
}

bool FindWadList( const bmodel_t* pModel, char*& pszWadList )
{
	assert( pModel );

	pszWadList = nullptr;

	if( pModel->entities )
	{
		char* pszNext = pModel->entities;

		while( true )
		{
			char* pszNextToken = COM_Parse( pszNext );

			if( !pszNextToken || !( *pszNextToken ) || com_token[ 0 ] == '}' )
			{
				break;
			}

			pszNext = pszNextToken;

			if( strcmp( com_token, "wad" ) == 0 )
			{
				COM_Parse( pszNextToken );

				//com_token is the wad path list - Solokiller
				size_t uiLength = strlen( com_token );

				const bool bHasSemiColonEnd = uiLength > 0 ? com_token[ uiLength - 1 ] == ';' : false;

				//Need to append a semicolon at the end so search operations are easier - Solokiller
				if( !bHasSemiColonEnd )
					++uiLength;

				pszWadList = new char[ uiLength + 1 ];

				strcpy( pszWadList, com_token );

				if( !bHasSemiColonEnd )
				{
					pszWadList[ uiLength - 1 ] = ';';
					pszWadList[ uiLength ] = '\0';
				}

				return true;
			}
		}
	}

	return false;
}

/*
================
BuildSurfaceDisplayList
================
*/
void BuildSurfaceDisplayList( bmodel_t* pModel, msurface_t *fa )
{
	int			i, lindex, lnumverts;
	Vector		local, transformed;
	medge_t		*pedges, *r_pedge;
	int			vertpage;
	Vector		*vec;
	float		s, t;

	//Regular brushes don't need multiple polygons - Solokiller
	if( fa->polys )
		return;

	// reconstruct the polygon
	pedges = pModel->edges;
	lnumverts = fa->numedges;
	vertpage = 0;

	//
	// draw texture
	//
	const size_t uiSize = sizeof( glpoly_t ) + ( lnumverts - 4 ) * VERTEXSIZE * sizeof( float );

	glpoly_t* poly = reinterpret_cast<glpoly_t*>( new byte[ uiSize ] );

	memset( poly, 0, uiSize );

	poly->next = fa->polys;
	poly->flags = fa->flags;
	fa->polys = poly;
	poly->numverts = lnumverts;

	for( i = 0; i<lnumverts; i++ )
	{
		lindex = pModel->surfedges[ fa->firstedge + i ];

		if( lindex > 0 )
		{
			r_pedge = &pedges[ lindex ];
			vec = &pModel->vertexes[ r_pedge->v[ 0 ] ].position;
		}
		else
		{
			r_pedge = &pedges[ -lindex ];
			vec = &pModel->vertexes[ r_pedge->v[ 1 ] ].position;
		}
		s = glm::dot( *vec, *reinterpret_cast<Vector*>( &fa->texinfo->vecs[ 0 ] ) ) + fa->texinfo->vecs[ 0 ][ 3 ];
		s /= fa->texinfo->texture->width;

		t = glm::dot( *vec, *reinterpret_cast<Vector*>( &fa->texinfo->vecs[ 1 ] ) ) + fa->texinfo->vecs[ 1 ][ 3 ];
		t /= fa->texinfo->texture->height;

		*reinterpret_cast<Vector*>( &poly->verts[ i ] ) = *vec;
		poly->verts[ i ][ 3 ] = s;
		poly->verts[ i ][ 4 ] = t;

		//
		// lightmap texture coordinates
		//
		s = glm::dot( *vec, *reinterpret_cast<Vector*>( &fa->texinfo->vecs[ 0 ] ) ) + fa->texinfo->vecs[ 0 ][ 3 ];
		s -= fa->texturemins[ 0 ];
		s += fa->light_s * 16;
		s += 8;
		s /= BLOCK_WIDTH * 16; //fa->texinfo->texture->width;

		t = glm::dot( *vec, *reinterpret_cast<Vector*>( &fa->texinfo->vecs[ 1 ] ) ) + fa->texinfo->vecs[ 1 ][ 3 ];
		t -= fa->texturemins[ 1 ];
		t += fa->light_t * 16;
		t += 8;
		t /= BLOCK_HEIGHT * 16; //fa->texinfo->texture->height;

		poly->verts[ i ][ 5 ] = s;
		poly->verts[ i ][ 6 ] = t;
	}

	//
	// remove co-linear points - Ed
	//
	if( !false/*gl_keeptjunctions.value*/ && !( fa->flags & SURF_UNDERWATER ) )
	{
		for( i = 0; i < lnumverts; ++i )
		{
			Vector v1, v2;
			Vector *prev, *cur, *next;

			prev = reinterpret_cast<Vector*>( &poly->verts[ ( i + lnumverts - 1 ) % lnumverts ] );
			cur = reinterpret_cast<Vector*>( &poly->verts[ i ] );
			next = reinterpret_cast<Vector*>( &poly->verts[ ( i + 1 ) % lnumverts ] );

			v1 = *cur - *prev;
			v1 = glm::normalize( v1 );
			v2 = *next - *prev;
			v2 = glm::normalize( v2 );

			// skip co-linear points
#define COLINEAR_EPSILON 0.001
			if( ( fabs( v1[ 0 ] - v2[ 0 ] ) <= COLINEAR_EPSILON ) &&
				( fabs( v1[ 1 ] - v2[ 1 ] ) <= COLINEAR_EPSILON ) &&
				( fabs( v1[ 2 ] - v2[ 2 ] ) <= COLINEAR_EPSILON ) )
			{
				int j;
				for( j = i + 1; j < lnumverts; ++j )
				{
					int k;
					for( k = 0; k < VERTEXSIZE; ++k )
						poly->verts[ j - 1 ][ k ] = poly->verts[ j ][ k ];
				}
				--lnumverts;
				// retry next vertex next time, which is now current vertex
				--i;
			}
		}
	}
	poly->numverts = lnumverts;

	CreatePoly( poly );
}

bool LoadBrushModel( bmodel_t* pModel, dheader_t* pHeader )
{
	assert( pModel );
	assert( pHeader );

	const int iVersion = LittleValue( pHeader->version );

	if( iVersion != BSPVERSION )
	{
		printf( "BSP::LoadBrushmodel: %s has wrong version number (%d should be %d)\n", 
				pModel->name, iVersion, BSPVERSION );
		return false;
	}

	// swap all the lumps

	for( size_t i = 0; i<sizeof( dheader_t ) / 4; ++i )
		( ( int* ) pHeader )[ i ] = LittleValue( ( ( int* ) pHeader )[ i ] );

	// load into heap

	if( !Mod_LoadVertexes( pModel, pHeader, &pHeader->lumps[ LUMP_VERTEXES ] ) )
		return false;

	if( !Mod_LoadEdges( pModel, pHeader, &pHeader->lumps[ LUMP_EDGES ] ) )
		return false;

	if( !Mod_LoadSurfedges( pModel, pHeader, &pHeader->lumps[ LUMP_SURFEDGES ] ) )
		return false;

	//half-life loads entities first and looks for the wad key - Solokiller
	if( !Mod_LoadEntities( pModel, pHeader, &pHeader->lumps[ LUMP_ENTITIES ] ) )
		return false;

	char* pszWadList = nullptr;

	if( !BSP::FindWadList( pModel, pszWadList ) )
	{
		//TODO: is this supposed to be an error? - Solokiller
		printf( "Couldn't find wad list!\n" );
		return false;
	}

	{
		const size_t uiLength = strlen( pszWadList );

		char* pszWad = pszWadList;

		while( pszWad && *pszWad )
		{
			char* pszNext = strchr( pszWad + 1, ';' );

			if( !pszNext )
				break;

			//Null terminate so the next operations succeeds.
			*pszNext = '\0';

			//TODO: fix slashes.
			//Strip path.
			char* pszWadName = strrchr( pszWad, '\\' );

			if( pszWadName )
				pszWad = pszWadName + 1;

			char* pszExt = strrchr( pszWad, '.' );

			//Null terminate for convenience.
			if( pszExt )
				*pszExt = '\0';

			g_WadManager.AddWad( pszWad );

			pszWad = pszNext + 1;
		}
	}

	delete[] pszWadList;

	if( !Mod_LoadTextures( pModel, pHeader, &pHeader->lumps[ LUMP_TEXTURES ] ) )
		return false;

	if( !Mod_LoadLighting( pModel, pHeader, &pHeader->lumps[ LUMP_LIGHTING ] ) )
		return false;

	if( !Mod_LoadPlanes( pModel, pHeader, &pHeader->lumps[ LUMP_PLANES ] ) )
		return false;

	if( !Mod_LoadTexinfo( pModel, pHeader, &pHeader->lumps[ LUMP_TEXINFO ] ) )
		return false;

	if( !Mod_LoadFaces( pModel, pHeader, &pHeader->lumps[ LUMP_FACES ] ) )
		return false;

	if( !Mod_LoadMarksurfaces( pModel, pHeader, &pHeader->lumps[ LUMP_MARKSURFACES ] ) )
		return false;

	if( !Mod_LoadVisibility( pModel, pHeader, &pHeader->lumps[ LUMP_VISIBILITY ] ) )
		return false;

	if( !Mod_LoadLeafs( pModel, pHeader, &pHeader->lumps[ LUMP_LEAFS ] ) )
		return false;

	if( !Mod_LoadNodes( pModel, pHeader, &pHeader->lumps[ LUMP_NODES ] ) )
		return false;

	if( !Mod_LoadClipnodes( pModel, pHeader, &pHeader->lumps[ LUMP_CLIPNODES ] ) )
		return false;

	if( !Mod_LoadSubmodels( pModel, pHeader, &pHeader->lumps[ LUMP_MODELS ] ) )
		return false;

	Mod_MakeHull0( pModel );

	pModel->numframes = 2;		// regular and alternate animation


	dmodel_t* bm;

	bmodel_t* pLoadModel;

	bmodel_t* pOriginal = pModel;

	//
	// set up the submodels (FIXME: this is confusing)
	//
	for( int i = 0; i<pModel->numsubmodels; ++i )
	{
		bm = &pModel->submodels[ i ];

		pModel->hulls[ 0 ].firstclipnode = bm->headnode[ 0 ];
		for( size_t j = 1; j<MAX_MAP_HULLS; j++ )
		{
			pModel->hulls[ j ].firstclipnode = bm->headnode[ j ];
			pModel->hulls[ j ].lastclipnode = pModel->numclipnodes - 1;
		}

		pModel->firstmodelsurface = bm->firstface;
		pModel->nummodelsurfaces = bm->numfaces;

		pModel->maxs = bm->maxs;
		pModel->mins = bm->mins;

		pModel->radius = RadiusFromBounds( pModel->mins, pModel->maxs );

		pModel->numleafs = bm->visleafs;

		if( i < pModel->numsubmodels - 1 )
		{	// duplicate the basic information
			char	name[ 10 ];

			sprintf( name, "*%i", i + 1 );
			pLoadModel = Mod_FindName( name );

			if( !pLoadModel )
				return false;

			*pLoadModel = *pModel;
			strcpy( pLoadModel->name, name );
			pModel = pLoadModel;
		}
	}

	pModel = pOriginal;

	bmodel_t* pCurrentModel;

	for( int i = 0; i<pModel->numsurfaces; i++ )
	{
		/*
		//GL_CreateSurfaceLightmap( m->surfaces + i );
		if( pModel->surfaces[ i ].flags & SURF_DRAWTURB )
		continue;
		#ifndef QUAKE2
		if( pModel->surfaces[ i ].flags & SURF_DRAWSKY )
		continue;
		#endif
		*/
		BuildSurfaceDisplayList( pModel, pModel->surfaces + i );
	}

	for( size_t j = 1; j<MAX_MOD_KNOWN; j++ )
	{
		pCurrentModel = &mod_known[ j ];
		if( !pCurrentModel->name[ 0 ] )
			break;
		for( int i = 0; i<pCurrentModel->numsurfaces; i++ )
		{
			/*
			//GL_CreateSurfaceLightmap( m->surfaces + i );
			if( pCurrentModel->surfaces[ i ].flags & SURF_DRAWTURB )
				continue;
#ifndef QUAKE2
			if( pCurrentModel->surfaces[ i ].flags & SURF_DRAWSKY )
				continue;
#endif
*/
			BuildSurfaceDisplayList( pCurrentModel, pCurrentModel->surfaces + i );
		}
	}

	return true;
}

void FreeModel( bmodel_t* pModel )
{
	if( !pModel )
		return;

	delete[] pModel->submodels;
	delete[] pModel->planes;
	delete[] pModel->leafs;
	delete[] pModel->vertexes;
	delete[] pModel->edges;
	delete[] pModel->nodes;
	delete[] pModel->texinfo;

	msurface_t* pSurface = pModel->surfaces;
	glpoly_t* pNextPoly;

	for( int iIndex = 0; iIndex < pModel->numsurfaces; ++iIndex, ++pSurface )
	{
		for( glpoly_t* pPoly = pSurface->polys; pPoly; pPoly = pNextPoly )
		{
			pNextPoly = pPoly->next;

			delete[] pPoly;
		}
	}

	delete[] pModel->surfaces;
	delete[] pModel->surfedges;
	//Don't delete, deleted later on - Solokiller
	//delete[] pModel->clipnodes;
	delete[] pModel->marksurfaces;

	//The textures themselves are managed by CTextureManager now, so don't delete them here. - Solokiller
	g_TextureManager.Shutdown();

	delete[] pModel->textures;
	delete[] pModel->visdata;
	delete[] pModel->lightdata;
	delete[] pModel->entities;

	//Delete the clipping hulls for the first 2 hulls only, since the 3rd is the same as the 2nd and the 4th isn't used. - Solokiller
	for( size_t uiIndex = 0; uiIndex < 2; ++uiIndex )
	{
		hull_t& hull = pModel->hulls[ uiIndex ];

		delete[] hull.clipnodes;
		//hull.planes is a pointer to memory managed by bmodel_t
	}

	memset( pModel, 0, sizeof( bmodel_t ) );
}
}