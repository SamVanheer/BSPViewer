#include <cmath>
#include <cstdio>
#include <memory>

#include "utility/ByteSwap.h"

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
	int		i, j, pixels, num, max, altmax;
	miptex_t	*mt;
	texture_t	*tx, *tx2;
	texture_t	*anims[ 10 ];
	texture_t	*altanims[ 10 ];

	byte* mod_base = reinterpret_cast<byte*>( pHeader );

	//No textures to load.
	if( !l->filelen )
	{
		pModel->textures = nullptr;
		return true;
	}

	//TODO: half-life BSPs store textures in a different manner. Rework this function to handle it - Solokiller
	return true;

	dmiptexlump_t* m = ( dmiptexlump_t* ) ( mod_base + l->fileofs );

	m->nummiptex = LittleValue( m->nummiptex );

	pModel->numtextures = m->nummiptex;
	pModel->textures = new texture_t*[ m->nummiptex ];

	for( i = 0; i<m->nummiptex; i++ )
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
		tx = reinterpret_cast<texture_t*>( new byte[ sizeof( texture_t ) + pixels ] );
		pModel->textures[ i ] = tx;

		memcpy( tx->name, mt->name, sizeof( tx->name ) );
		tx->width = mt->width;
		tx->height = mt->height;
		for( j = 0; j<MIPLEVELS; j++ )
			tx->offsets[ j ] = mt->offsets[ j ] + sizeof( texture_t ) - sizeof( miptex_t );
		// the pixels immediately follow the structures
		memcpy( tx + 1, mt + 1, pixels );


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

	//
	// sequence the animations
	//
	for( i = 0; i<m->nummiptex; i++ )
	{
		tx = pModel->textures[ i ];
		if( !tx || tx->name[ 0 ] != '+' )
			continue;
		if( tx->anim_next )
			continue;	// allready sequenced

						// find the number of frames in the animation
		memset( anims, 0, sizeof( anims ) );
		memset( altanims, 0, sizeof( altanims ) );

		max = tx->name[ 1 ];
		altmax = 0;
		if( max >= 'a' && max <= 'z' )
			max -= 'a' - 'A';
		if( max >= '0' && max <= '9' )
		{
			max -= '0';
			altmax = 0;
			anims[ max ] = tx;
			max++;
		}
		else if( max >= 'A' && max <= 'J' )
		{
			altmax = max - 'A';
			max = 0;
			altanims[ altmax ] = tx;
			altmax++;
		}
		else
		{
			printf( "Bad animating texture %s\n", tx->name );
			return false;
		}

		for( j = i + 1; j<m->nummiptex; j++ )
		{
			tx2 = pModel->textures[ j ];
			if( !tx2 || tx2->name[ 0 ] != '+' )
				continue;
			if( strcmp( tx2->name + 2, tx->name + 2 ) )
				continue;

			num = tx2->name[ 1 ];
			if( num >= 'a' && num <= 'z' )
				num -= 'a' - 'A';
			if( num >= '0' && num <= '9' )
			{
				num -= '0';
				anims[ num ] = tx2;
				if( num + 1 > max )
					max = num + 1;
			}
			else if( num >= 'A' && num <= 'J' )
			{
				num = num - 'A';
				altanims[ num ] = tx2;
				if( num + 1 > altmax )
					altmax = num + 1;
			}
			else
			{
				printf( "Bad animating texture %s\n", tx->name );
				return false;
			}
		}

#define	ANIM_CYCLE	2
		// link them all together
		for( j = 0; j<max; j++ )
		{
			tx2 = anims[ j ];
			if( !tx2 )
			{
				printf( "Missing frame %i of %s\n", j, tx->name );
				return false;
			}
			tx2->anim_total = max * ANIM_CYCLE;
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = ( j + 1 ) * ANIM_CYCLE;
			tx2->anim_next = anims[ ( j + 1 ) % max ];
			if( altmax )
				tx2->alternate_anims = altanims[ 0 ];
		}
		for( j = 0; j<altmax; j++ )
		{
			tx2 = altanims[ j ];
			if( !tx2 )
			{
				printf( "Missing frame %i of %s\n", j, tx->name );
				return false;
			}
			tx2->anim_total = altmax * ANIM_CYCLE;
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = ( j + 1 ) * ANIM_CYCLE;
			tx2->anim_next = altanims[ ( j + 1 ) % altmax ];
			if( max )
				tx2->alternate_anims = anims[ 0 ];
		}
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

void CalcSurfaceExtents( msurface_t* pSurf )
{
	//TODO: implement
}

void GL_SubdivideSurface( msurface_t* pSurf )
{
	//TODO: implement
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

		CalcSurfaceExtents( out );

		// lighting info

		for( i = 0; i<MAXLIGHTMAPS; i++ )
			out->styles[ i ] = in->styles[ i ];
		i = LittleValue( in->lightofs );
		if( i == -1 )
			out->samples = NULL;
		else
			out->samples = pModel->lightdata + i;

		// set the drawing flags flag

		if( !strncmp( out->texinfo->texture->name, "sky", 3 ) )	// sky
		{
			out->flags |= ( SURF_DRAWSKY | SURF_DRAWTILED );
#ifndef QUAKE2
			GL_SubdivideSurface( out );	// cut up polygon for warps
#endif
			continue;
		}

		if( !strncmp( out->texinfo->texture->name, "*", 1 ) )		// turbulent
		{
			out->flags |= ( SURF_DRAWTURB | SURF_DRAWTILED );
			for( i = 0; i<2; i++ )
			{
				out->extents[ i ] = 16384;
				out->texturemins[ i ] = -8192;
			}
			GL_SubdivideSurface( out );	// cut up polygon for warps
			continue;
		}

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

#define MAX_MOD_KNOWN 512

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
		return false;
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

	if( !Mod_LoadEntities( pModel, pHeader, &pHeader->lumps[ LUMP_ENTITIES ] ) )
		return false;

	if( !Mod_LoadSubmodels( pModel, pHeader, &pHeader->lumps[ LUMP_MODELS ] ) )
		return false;

	Mod_MakeHull0( pModel );

	pModel->numframes = 2;		// regular and alternate animation


	dmodel_t* bm;

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
			pModel = Mod_FindName( name );
			*pModel = *pModel;
			strcpy( pModel->name, name );
			pModel = pModel;
		}
	}

	return true;
}
}