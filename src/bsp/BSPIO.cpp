/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
****/
#include <cassert>
#include <cstdio>
#include <memory>

#include "utility/ByteSwap.h"

#include "BSPIO.h"

template<typename DATA>
bool CopyLump( const dheader_t* const pHeader, const BSPLump lump, DATA& data )
{
	assert( pHeader );
	assert( LUMP_FIRST <= lump && lump <= LUMP_LAST );

	const int iLength = pHeader->lumps[ lump ].filelen;
	const int iOfs = pHeader->lumps[ lump ].fileofs;

	if( iLength % sizeof( DATA::Type_t ) )
	{
		printf( "Irregular length encountered while loading lump %d (%u byte chunk, alignment is %u)\n", lump, iLength % sizeof( DATA::Type_t ), sizeof( DATA::Type_t ) );
		return false;
	}

	//Total size exceeds destination size.
	if( iLength > ( sizeof( DATA::Type_t ) * DATA::MAX_SIZE ) )
	{
		printf( "Source data too large while loading lump %d (max: %u, actual: %d)\n", lump, sizeof( DATA::Type_t ) * DATA::MAX_SIZE, iLength );
		return false;
	}

	memcpy( data.data, reinterpret_cast<const byte* const>( pHeader ) + iOfs, iLength );

	return true;
}

int FastChecksum( const void* const buffer, int bytes )
{
	int	checksum = 0;

	const char* pBuffer = reinterpret_cast<const char*>( buffer );

	while( bytes-- )
		checksum = _rotl( checksum, 4 ) ^ *pBuffer++;

	return checksum;
}

template<typename DATA>
void FastBSPDataChecksum( DATA& data )
{
	data.checksum = FastChecksum( data.data, data.count * sizeof( DATA::Type_t ) );
}

bool LoadBSPFile( const char* const pszFileName, CBSPFile& file )
{
	assert( pszFileName );

	//That's one big chunk of memory.
	memset( &file, 0, sizeof( file ) );

	FILE* pFile = fopen( pszFileName, "rb" );

	if( !pFile )
	{
		printf( "Couldn't open BSP file \"%s\"\n", pszFileName );
		return false;
	}

	fseek( pFile, 0, SEEK_END );

	const size_t size = ftell( pFile );

	fseek( pFile, 0, SEEK_SET );

	{
		std::unique_ptr<dheader_t> header( reinterpret_cast<dheader_t*>( new byte[ size ] ) );

		dheader_t* pHeader = header.get();

		const size_t readCount = fread( header.get(), 1, size, pFile );

		fclose( pFile );

		if( readCount != size )
		{
			printf( "Error reading BSP file \"%s\": expected %u bytes, read %u\n", pszFileName, size, readCount );
			return false;
		}

		for( size_t uiIndex = 0; uiIndex < sizeof( dheader_t ) / 4; ++uiIndex )
		{
			reinterpret_cast<int*>( pHeader )[ uiIndex ] = LittleValue( reinterpret_cast<int*>( pHeader )[ uiIndex ] );
		}

		if( pHeader->version != BSPVERSION )
		{
			printf( "BSP file \"%s\" is version %d, not %d\n", pszFileName, pHeader->version, BSPVERSION );
			return false;
		}

		bool bSuccess = true;

		bSuccess = CopyLump( pHeader, LUMP_MODELS, file.models ) && bSuccess;
		bSuccess = CopyLump( pHeader, LUMP_VERTEXES, file.vertexes ) && bSuccess;
		bSuccess = CopyLump( pHeader, LUMP_PLANES, file.planes ) && bSuccess;
		bSuccess = CopyLump( pHeader, LUMP_LEAFS, file.leafs ) && bSuccess;
		bSuccess = CopyLump( pHeader, LUMP_NODES, file.nodes ) && bSuccess;
		bSuccess = CopyLump( pHeader, LUMP_TEXINFO, file.texinfo ) && bSuccess;
		bSuccess = CopyLump( pHeader, LUMP_CLIPNODES, file.clipnodes ) && bSuccess;
		bSuccess = CopyLump( pHeader, LUMP_FACES, file.faces ) && bSuccess;
		bSuccess = CopyLump( pHeader, LUMP_MARKSURFACES, file.marksurfaces ) && bSuccess;
		bSuccess = CopyLump( pHeader, LUMP_SURFEDGES, file.surfedges ) && bSuccess;
		bSuccess = CopyLump( pHeader, LUMP_EDGES, file.edges ) && bSuccess;

		bSuccess = CopyLump( pHeader, LUMP_TEXTURES, file.texdata ) && bSuccess;
		bSuccess = CopyLump( pHeader, LUMP_VISIBILITY, file.visdata ) && bSuccess;
		bSuccess = CopyLump( pHeader, LUMP_LIGHTING, file.lightdata ) && bSuccess;
		bSuccess = CopyLump( pHeader, LUMP_ENTITIES, file.entdata ) && bSuccess;

		if( !bSuccess )
		{
			printf( "Failed to read BSP lump\n" );
			return false;
		}
	}

	SwapBSPFile( false, file );

	FastBSPDataChecksum( file.models );
	FastBSPDataChecksum( file.vertexes );
	FastBSPDataChecksum( file.planes );
	FastBSPDataChecksum( file.leafs );
	FastBSPDataChecksum( file.nodes );
	FastBSPDataChecksum( file.texinfo );
	FastBSPDataChecksum( file.clipnodes );
	FastBSPDataChecksum( file.faces );
	FastBSPDataChecksum( file.surfedges );
	FastBSPDataChecksum( file.edges );
	FastBSPDataChecksum( file.texdata );
	FastBSPDataChecksum( file.visdata );
	FastBSPDataChecksum( file.lightdata );
	FastBSPDataChecksum( file.entdata );

	return true;
}

/*
=============
SwapBSPFile

Byte swaps all data in a bsp file.
=============
*/
void SwapBSPFile( const bool todisk, CBSPFile& file )
{
	int				i, j, c;
	dmodel_t		*d;
	dmiptexlump_t	*mtl;


	// models	
	for( i = 0; i<file.models.count; i++ )
	{
		d = &file.models.data[ i ];

		for( j = 0; j<MAX_MAP_HULLS; j++ )
			d->headnode[ j ] = LittleValue( d->headnode[ j ] );

		d->visleafs = LittleValue( d->visleafs );
		d->firstface = LittleValue( d->firstface );
		d->numfaces = LittleValue( d->numfaces );

		for( j = 0; j<3; j++ )
		{
			d->mins[ j ] = LittleValue( d->mins[ j ] );
			d->maxs[ j ] = LittleValue( d->maxs[ j ] );
			d->origin[ j ] = LittleValue( d->origin[ j ] );
		}
	}

	//
	// vertexes
	//
	for( i = 0; i<file.vertexes.count; i++ )
	{
		for( j = 0; j<3; j++ )
			file.vertexes.data[ i ].point[ j ] = LittleValue( file.vertexes.data[ i ].point[ j ] );
	}

	//
	// planes
	//	
	for( i = 0; i<file.planes.count; i++ )
	{
		for( j = 0; j<3; j++ )
			file.planes.data[ i ].normal[ j ] = LittleValue( file.planes.data[ i ].normal[ j ] );
		file.planes.data[ i ].dist = LittleValue( file.planes.data[ i ].dist );
		file.planes.data[ i ].type = LittleValue( file.planes.data[ i ].type );
	}

	//
	// texinfos
	//	
	for( i = 0; i<file.texinfo.count; i++ )
	{
		for( j = 0; j<8; j++ )
			file.texinfo.data[ i ].vecs[ 0 ][ j ] = LittleValue( file.texinfo.data[ i ].vecs[ 0 ][ j ] );
		file.texinfo.data[ i ].miptex = LittleValue( file.texinfo.data[ i ].miptex );
		file.texinfo.data[ i ].flags = LittleValue( file.texinfo.data[ i ].flags );
	}

	//
	// faces
	//
	for( i = 0; i<file.faces.count; i++ )
	{
		file.faces.data[ i ].texinfo = LittleValue( file.faces.data[ i ].texinfo );
		file.faces.data[ i ].planenum = LittleValue( file.faces.data[ i ].planenum );
		file.faces.data[ i ].side = LittleValue( file.faces.data[ i ].side );
		file.faces.data[ i ].lightofs = LittleValue( file.faces.data[ i ].lightofs );
		file.faces.data[ i ].firstedge = LittleValue( file.faces.data[ i ].firstedge );
		file.faces.data[ i ].numedges = LittleValue( file.faces.data[ i ].numedges );
	}

	//
	// nodes
	//
	for( i = 0; i<file.nodes.count; i++ )
	{
		file.nodes.data[ i ].planenum = LittleValue( file.nodes.data[ i ].planenum );
		for( j = 0; j<3; j++ )
		{
			file.nodes.data[ i ].mins[ j ] = LittleValue( file.nodes.data[ i ].mins[ j ] );
			file.nodes.data[ i ].maxs[ j ] = LittleValue( file.nodes.data[ i ].maxs[ j ] );
		}
		file.nodes.data[ i ].children[ 0 ] = LittleValue( file.nodes.data[ i ].children[ 0 ] );
		file.nodes.data[ i ].children[ 1 ] = LittleValue( file.nodes.data[ i ].children[ 1 ] );
		file.nodes.data[ i ].firstface = LittleValue( file.nodes.data[ i ].firstface );
		file.nodes.data[ i ].numfaces = LittleValue( file.nodes.data[ i ].numfaces );
	}

	//
	// leafs
	//
	for( i = 0; i<file.leafs.count; i++ )
	{
		file.leafs.data[ i ].contents = LittleValue( file.leafs.data[ i ].contents );
		for( j = 0; j<3; j++ )
		{
			file.leafs.data[ i ].mins[ j ] = LittleValue( file.leafs.data[ i ].mins[ j ] );
			file.leafs.data[ i ].maxs[ j ] = LittleValue( file.leafs.data[ i ].maxs[ j ] );
		}

		file.leafs.data[ i ].firstmarksurface = LittleValue( file.leafs.data[ i ].firstmarksurface );
		file.leafs.data[ i ].nummarksurfaces = LittleValue( file.leafs.data[ i ].nummarksurfaces );
		file.leafs.data[ i ].visofs = LittleValue( file.leafs.data[ i ].visofs );
	}

	//
	// clipnodes
	//
	for( i = 0; i<file.clipnodes.count; i++ )
	{
		file.clipnodes.data[ i ].planenum = LittleValue( file.clipnodes.data[ i ].planenum );
		file.clipnodes.data[ i ].children[ 0 ] = LittleValue( file.clipnodes.data[ i ].children[ 0 ] );
		file.clipnodes.data[ i ].children[ 1 ] = LittleValue( file.clipnodes.data[ i ].children[ 1 ] );
	}

	//
	// miptex
	//
	if( file.texdata.count )
	{
		mtl = ( dmiptexlump_t * ) file.texdata.data;
		if( todisk )
			c = mtl->nummiptex;
		else
			c = LittleValue( mtl->nummiptex );
		mtl->nummiptex = LittleValue( mtl->nummiptex );
		for( i = 0; i<c; i++ )
			mtl->dataofs[ i ] = LittleValue( mtl->dataofs[ i ] );
	}

	//
	// marksurfaces
	//
	for( i = 0; i<file.marksurfaces.count; i++ )
		file.marksurfaces.data[ i ] = LittleValue( file.marksurfaces.data[ i ] );

	//
	// surfedges
	//
	for( i = 0; i<file.surfedges.count; i++ )
		file.surfedges.data[ i ] = LittleValue( file.surfedges.data[ i ] );

	//
	// edges
	//
	for( i = 0; i<file.edges.count; i++ )
	{
		file.edges.data[ i ].v[ 0 ] = LittleValue( file.edges.data[ i ].v[ 0 ] );
		file.edges.data[ i ].v[ 1 ] = LittleValue( file.edges.data[ i ].v[ 1 ] );
	}
}