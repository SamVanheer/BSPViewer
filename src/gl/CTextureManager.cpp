#include <cassert>
#include <cstdio>
#include <cstring>

#include "wad/CWadManager.h"

#include "GLMiptex.h"

#include "CShaderManager.h"

#include "CTextureManager.h"

CTextureManager g_TextureManager;

bool CTextureManager::Initialize( const size_t uiNumTextures )
{
	//Forgot to call Shutdown.
	if( m_bInitialized )
	{
		printf( "CTextureManager::Initialize: Manager was not shut down!\n" );
		Shutdown();
	}

	m_bInitialized = true;

	//Preallocate space for all used textures.
	m_Textures.resize( uiNumTextures );

	//Zero out the memory.
	memset( m_Textures.data(), 0, sizeof( texture_t ) * m_Textures.size() );

	return true;
}

void CTextureManager::Shutdown()
{
	if( !m_bInitialized )
		return;

	m_bInitialized = false;

	m_TexMap.clear();

	//Force clear the memory used by the map.
	m_TexMap.swap( TexMap_t() );

	//Free all textures.
	for( auto& tex : m_Textures )
	{
		glDeleteTextures( 1, &tex.gl_texturenum );
	}

	m_Textures.clear();
	m_Textures.shrink_to_fit();

	m_uiTexturesInUse = 0;
}

const texture_t* CTextureManager::FindTexture( const char* const pszName ) const
{
	assert( pszName );

	if( !pszName )
		return nullptr;

	auto it = m_TexMap.find( pszName );

	if( it != m_TexMap.end() )
		return &m_Textures[ it->second ];

	return nullptr;
}

texture_t* CTextureManager::FindTexture( const char* const pszName )
{
	return const_cast<texture_t*>( const_cast<const CTextureManager*>( this )->FindTexture( pszName ) );
}

texture_t* CTextureManager::LoadTexture( const char* const pszName, const miptex_t* pMiptex )
{
	assert( pszName );

	if( !pszName )
		return nullptr;

	if( auto pTexture = FindTexture( pszName ) )
		return pTexture;

	const size_t uiLength = strlen( pszName );

	//Should never happen since all textures come from lumps.
	if( uiLength >= WAD_MAX_LUMP_NAME_SIZE )
	{
		printf( "CTextureManager::LoadTexture: Texture name too long (max %u, got %u)\n", WAD_MAX_LUMP_NAME_SIZE, uiLength );
		return nullptr;
	}

	if( m_uiTexturesInUse >= m_Textures.size() )
	{
		printf( "CTextureManager::LoadTexture: Out of texture IDs (max: %u)\n", m_Textures.size() );
		return nullptr;
	}

	if( !pMiptex )
		pMiptex = g_WadManager.FindTextureByName( pszName );

	if( !pMiptex )
	{
		printf( "CTextureManager::LoadTexture: Couldn't find texture \"%s\"\n", pszName );
		return nullptr;
	}

	GLuint tex = UploadMiptex( pMiptex );

	if( tex == 0 )
		return nullptr;

	const size_t uiIndex = m_uiTexturesInUse;

	texture_t* pTexture = &m_Textures[ uiIndex ];

	//Length checked earlier.
	strcpy( pTexture->name, pszName );

	pTexture->width = pMiptex->width;
	pTexture->height = pMiptex->height;

	pTexture->gl_texturenum = tex;

	const char* pszShaderName = "LightMappedGeneric";

	if( pszName[ 0 ] == '{' )
		pszShaderName = "LightMappedAlphaTest";
	else if( pszName[ 0 ] == '!' )
		pszShaderName = "LightMappedWater";

	pTexture->pShader = g_ShaderManager.GetShader( pszShaderName );

	if( !pTexture->pShader )
	{
		printf( "Shader \"%s\" not found, falling back to generic\n", pszName );
		pTexture->pShader = g_ShaderManager.GetShader( "LightMappedGeneric" );
	}

	auto result = m_TexMap.insert( std::make_pair( pTexture->name, uiIndex ) );

	if( !result.second )
	{
		//Insertion failed; remove texture.
		printf( "CTextureManager::LoadTexture: Failed to insert texture \"%s\" into map\n", pszName );
		glDeleteTextures( 1, &pTexture->gl_texturenum );

		memset( pTexture, 0, sizeof( texture_t ) );
		return nullptr;
	}

	++m_uiTexturesInUse;

	return pTexture;
}

//TODO: define this elsewhere - Solokiller
#define	ANIM_CYCLE	2

bool CTextureManager::SetupAnimatingTextures()
{
	texture_t* anims[ 10 ];
	texture_t* altanims[ 10 ];

	int max, altmax, num;

	//
	// sequence the animations
	//
	for( size_t uiIndex = 0; uiIndex < m_uiTexturesInUse; ++uiIndex )
	{
		auto& texture = m_Textures[ uiIndex ];

		if( texture.name[ 0 ] != '+' )
			continue;
		if( texture.anim_next )
			continue;	// allready sequenced

		// find the number of frames in the animation
		memset( anims, 0, sizeof( anims ) );
		memset( altanims, 0, sizeof( altanims ) );

		max = texture.name[ 1 ];

		altmax = 0;

		if( max >= 'a' && max <= 'z' )
			max -= 'a' - 'A';
		if( max >= '0' && max <= '9' )
		{
			max -= '0';
			altmax = 0;
			anims[ max ] = &texture;
			++max;
		}
		else if( max >= 'A' && max <= 'J' )
		{
			altmax = max - 'A';
			max = 0;
			altanims[ altmax ] = &texture;
			++altmax;
		}
		else
		{
			printf( "Bad animating texture %s\n", texture.name );
			return false;
		}

		for( size_t uiInner = uiIndex + 1; uiInner < m_uiTexturesInUse; ++uiInner )
		{
			auto& texture2 = m_Textures[ uiInner ];

			if( texture2.name[ 0 ] != '+' )
				continue;
			if( strcmp( texture2.name + 2, texture.name + 2 ) )
				continue;

			num = texture2.name[ 1 ];

			if( num >= 'a' && num <= 'z' )
				num -= 'a' - 'A';
			if( num >= '0' && num <= '9' )
			{
				num -= '0';
				anims[ num ] = &texture2;
				if( num + 1 > max )
					max = num + 1;
			}
			else if( num >= 'A' && num <= 'J' )
			{
				num = num - 'A';
				altanims[ num ] = &texture2;
				if( num + 1 > altmax )
					altmax = num + 1;
			}
			else
			{
				printf( "Bad animating texture %s\n", texture.name );
				return false;
			}
		}

		// link them all together
		for( int j = 0; j < max; ++j )
		{
			auto texture2 = anims[ j ];
			if( !texture2 )
			{
				printf( "Missing frame %i of %s\n", j, texture.name );
				return false;
			}

			texture2->anim_total = max * ANIM_CYCLE;
			texture2->anim_min = j * ANIM_CYCLE;
			texture2->anim_max = ( j + 1 ) * ANIM_CYCLE;
			texture2->anim_next = anims[ ( j + 1 ) % max ];
			if( altmax )
				texture2->alternate_anims = altanims[ 0 ];
		}

		for( int j = 0; j < altmax; ++j )
		{
			auto texture2 = altanims[ j ];
			if( !texture2 )
			{
				printf( "Missing frame %i of %s\n", j, texture.name );
				return false;
			}

			texture2->anim_total = altmax * ANIM_CYCLE;
			texture2->anim_min = j * ANIM_CYCLE;
			texture2->anim_max = ( j + 1 ) * ANIM_CYCLE;
			texture2->anim_next = altanims[ ( j + 1 ) % altmax ];
			if( max )
				texture2->alternate_anims = anims[ 0 ];
		}
	}

	return true;
}