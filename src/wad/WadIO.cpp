#include <cassert>
#include <cctype>
#include <cstdio>
#include <memory>

#include "common/Const.h"
#include "utility/ByteSwap.h"

#include "WadIO.h"

wadinfo_t* LoadWadFile( const char* const pszFileName )
{
	assert( pszFileName );

	FILE* pFile = fopen( pszFileName, "rb" );

	if( !pFile )
	{
		printf( "LoadWadFile: Couldn't open WAD \"%s\"\n", pszFileName );
		return nullptr;
	}

	fseek( pFile, 0, SEEK_END );

	const size_t size = ftell( pFile );

	fseek( pFile, 0, SEEK_SET );

	wadinfo_t* pWad = reinterpret_cast<wadinfo_t*>( new byte[ size ] );

	const size_t read = fread( pWad, 1, size, pFile );

	fclose( pFile );

	if( read != size )
	{
		printf( "LoadWadFile: Failed to read WAD file \"%s\" (expected %u, got %u)\n", pszFileName, size, read );
		delete[] pWad;

		return nullptr;
	}

	if( strncmp( WAD2_ID, pWad->identification, 4 ) && strncmp( WAD3_ID, pWad->identification, 4 ) )
	{
		printf( "LoadWadFile: File \"%s\" is not a WAD2 or WAD3 file\n", pszFileName );
		delete[] pWad;
		return nullptr;
	}

	pWad->infotableofs = LittleValue( pWad->infotableofs );
	pWad->numlumps = LittleValue( pWad->numlumps );

	lumpinfo_t* pLump = reinterpret_cast<lumpinfo_t*>( reinterpret_cast<byte*>( pWad ) + pWad->infotableofs );

	//Swap all variables, clean up names.
	for( int iLump = 0; iLump < pWad->numlumps; ++iLump, ++pLump )
	{
		CleanupWadLumpName( pLump->name, pLump->name, sizeof( pLump->name ) );

		pLump->filepos		= LittleValue( pLump->filepos );
		pLump->disksize		= LittleValue( pLump->disksize );
		pLump->size			= LittleValue( pLump->size );
	}

	return pWad;
}

void CleanupWadLumpName( const char* in, char* out, const size_t uiBufferSize )
{
	//Must be large enough to fit at least an entire name.
	assert( uiBufferSize >= WAD_MAX_LUMP_NAME_SIZE );

	size_t i;

	for( i = 0; i< WAD_MAX_LUMP_NAME_SIZE; i++ )
	{
		if( !in[ i ] )
			break;

		out[ i ] = toupper( in[ i ] );
	}

	//Fill the rest of the buffer with 0.
	memset( out + i, 0, uiBufferSize - i );
}

const miptex_t* Wad_FindTexture( const wadinfo_t* pWad, const char* const pszName )
{
	assert( pWad );
	assert( pszName );

	if( !pWad || !pszName )
		return nullptr;

	const lumpinfo_t* pLump = reinterpret_cast<const lumpinfo_t*>( reinterpret_cast<const byte*>( pWad ) + pWad->infotableofs );

	for( int iLump = 0; iLump < pWad->numlumps; ++iLump, ++pLump )
	{
		//Found it.
		if( _stricmp( pszName, pLump->name ) == 0 )
		{
			//Must be a miptex lump.
			if( pLump->type != ( TYP_LUMPY + TYP_LUMPY_MIPTEX ) )
			{
				return nullptr;
			}

			//Compression isn't used, so just return it.
			return reinterpret_cast<const miptex_t*>( reinterpret_cast<const byte*>( pWad ) + pLump->filepos );
		}
	}

	return nullptr;
}