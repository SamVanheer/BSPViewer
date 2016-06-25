#ifndef WAD_CWADFILE_H
#define WAD_CWADFILE_H

#include <cassert>
#include <cstring>
#include <memory>

#include "core/Platform.h"
#include "common/Const.h"

#include "WadFile.h"

/**
*	Lightweight wrapper around a wad file.
*/
class CWadFile final
{
public:
	/**
	*	Constructor.
	*	@param pszFilename Name of the wad. Excluding path and extension.
	*	@param pWad Wad to take ownership of. The wad will be freed unless ownership is released by calling Release.
	*	@see Release
	*/
	CWadFile( const char* const pszFilename, wadinfo_t* pWad )
		: m_pWad( pWad )
	{
		strncpy( m_szFilename, pszFilename, sizeof( m_szFilename ) );

		m_szFilename[ sizeof( m_szFilename ) - 1 ] = '\0';
	}

	/**
	*	Destructor.
	*/
	~CWadFile()
	{
		delete[] m_pWad;
	}

	/**
	*	@return Whether this wad file is still valid.
	*/
	bool IsValid() const { return m_pWad != nullptr; }

	/**
	*	@return Filename of this wad.
	*/
	const char* GetFilename() const { return m_szFilename; }

	/**
	*	@return The wad file.
	*/
	const wadinfo_t* Get() const { return m_pWad; }

	/**
	*	Releases ownership of the wad file.
	*	@return The wad file, or nullptr if ownership was already released.
	*/
	wadinfo_t* Release()
	{
		memset( m_szFilename, 0, sizeof( m_szFilename ) );

		wadinfo_t* pWad = m_pWad;

		m_pWad = nullptr;

		return pWad;
	}

	/**
	*	@return Number of lumps in this wad.
	*/
	int GetNumLumps() const { return m_pWad->numlumps; }

	/**
	*	Gets the array of lumps.
	*	@return Lump array.
	*/
	const lumpinfo_t* GetLumps() const
	{
		return reinterpret_cast<const lumpinfo_t*>( reinterpret_cast<byte*>( m_pWad ) + m_pWad->infotableofs );
	}

	/**
	*	Gets the requested lump by index.
	*	@param iLump The index of the requested lump.
	*	@return Requested lump, or null if the given lump does not exist.
	*/
	const lumpinfo_t* GetLumpByIndex( const int iLump ) const
	{
		assert( m_pWad );
		assert( iLump >= 0 && iLump < m_pWad->numlumps );

		if( iLump < 0 || iLump >= m_pWad->numlumps )
			return nullptr;

		return &GetLumps()[ iLump ];
	}

	/**
	*	Gets the requested lump by name.
	*	@param pszName The name of the requested lump.
	*	@param iLumpType If not TYP_NONE, check if the lump is the given type first.
	*	@return Requested lump, or null if the given lump does not exist.
	*/
	const lumpinfo_t* GetLumpByName( const char* const pszName, const int iLumpType = TYP_NONE ) const
	{
		assert( m_pWad );
		assert( pszName );

		const lumpinfo_t* pLump = GetLumps();

		for( int iLump = 0; iLump < m_pWad->numlumps; ++iLump, ++pLump )
		{
			if( _stricmp( pszName, pLump->name ) == 0 )
			{
				if( iLumpType != TYP_NONE )
				{
					if( pLump->type != iLumpType )
						return nullptr;
				}

				return pLump;
			}
		}

		return nullptr;
	}

	/**
	*	Gets the data pointed to by the given lump.
	*	@return Lump data, or null if the lump is invalid.
	*/
	const void* GetLumpData( const lumpinfo_t* pLump ) const
	{
		assert( m_pWad );
		assert( pLump );

		if( !pLump )
			return nullptr;

		//Check if the lump is actually from this wad.
		if( pLump < GetLumps() || pLump >= GetLumps() + GetNumLumps() )
			return nullptr;

		return reinterpret_cast<const byte*>( m_pWad ) + pLump->filepos;
	}

private:
	char m_szFilename[ MAX_PATH_LENGTH ];
	wadinfo_t* m_pWad;

private:
	CWadFile( const CWadFile& ) = delete;
	CWadFile& operator=( const CWadFile& ) = delete;
};

#endif //WAD_CWADFILE_H