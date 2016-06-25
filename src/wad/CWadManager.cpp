#include <algorithm>
#include <cassert>

#include "CWadFile.h"
#include "WadIO.h"

#include "CWadManager.h"

CWadManager g_WadManager;

CWadManager::~CWadManager()
{
	Clear();
}

void CWadManager::SetBasePath( const char* const pszBasePath )
{
	assert( pszBasePath );

	if( !pszBasePath )
		return;

	strncpy( m_szBasePath, pszBasePath, sizeof( m_szBasePath ) );

	m_szBasePath[ sizeof( m_szBasePath ) -1 ] = '\0';
}

const CWadFile* CWadManager::FindWadByName( const char* const pszWadName ) const
{
	assert( pszWadName );

	if( !pszWadName )
		return nullptr;

	auto it = std::find_if( m_WadFiles.begin(), m_WadFiles.end(), 
	[ = ]( const auto& wad )
	{
		return strcasecmp( pszWadName, wad->GetFilename() ) == 0;
	}
	);

	return it != m_WadFiles.end() ? it->get() : nullptr;
}

const miptex_t* CWadManager::FindTextureByName( const char* const pszTextureName ) const
{
	assert( pszTextureName );

	if( !pszTextureName )
		return nullptr;

	for( const auto& wad : m_WadFiles )
	{
		if( auto pLump = wad->GetLumpByName( pszTextureName, TYP_LUMPY + TYP_LUMPY_MIPTEX ) )
		{
			if( auto pTexture = reinterpret_cast<const miptex_t*>( wad->GetLumpData( pLump ) ) )
				return pTexture;
		}
	}

	return nullptr;
}

CWadManager::AddResult CWadManager::AddWad( const char* const pszWadName )
{
	assert( pszWadName );

	if( !pszWadName || !( *pszWadName ) )
		return AddResult::INVALID_NAME;

	if( FindWadByName( pszWadName ) )
		return AddResult::ALREADY_ADDED;

	char szPath[ MAX_PATH_LENGTH ];

	const int iResult = snprintf( szPath, sizeof( szPath ), "%s/%s%s", m_szBasePath, pszWadName, WAD_FILE_EXT );

	if( iResult < 0 || static_cast<size_t>( iResult ) >= sizeof( szPath ) )
		return AddResult::INVALID_NAME;

	auto pWad = LoadWadFile( szPath );

	//TODO: could've been an I/O error - Solokiller
	if( !pWad )
		return AddResult::FILE_NOT_FOUND;

	m_WadFiles.emplace_back( std::make_unique<CWadFile>( pszWadName, pWad ) );

	return AddResult::SUCCESS;
}

void CWadManager::Clear()
{
	m_WadFiles.clear();
	m_WadFiles.shrink_to_fit();
}