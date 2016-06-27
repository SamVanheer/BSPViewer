#include <cstring>

#include "bsp/BSPRenderIO.h"

#include "CBaseEntity.h"

void CBaseEntity::Construct( const char* const pszClassName, const size_t uiEntIndex )
{
	m_szClassName = pszClassName;
	m_uiEntIndex = uiEntIndex;
}

void CBaseEntity::OnCreate()
{
}

void CBaseEntity::OnDestroy()
{
}

void ParseVector( const char* pszString, Vector& vec )
{
	size_t uiIndex;

	char* pszNext = const_cast<char*>( pszString );

	for( uiIndex = 0; uiIndex < 3; ++uiIndex )
	{
		vec[ uiIndex ] = static_cast<float>( strtod( pszNext, &pszNext ) );

		if( !( *pszNext ) )
			break;
	}

	for( ; uiIndex < 3; ++uiIndex )
	{
		vec[ uiIndex ] = 0;
	}
}

template<typename T>
T clamp( T min, T val, T max )
{
	return ( val < min ) ? min : ( val > max ? max : val );
}

bool CBaseEntity::KeyValue( const char* pszKey, const char* pszValue )
{
	if( strcmp( "origin", pszKey ) == 0 )
	{
		ParseVector( pszValue, m_vecOrigin );
		return true;
	}
	else if( strcmp( "angles", pszKey ) == 0 )
	{
		ParseVector( pszValue, m_vecAngles );
		return true;
	}
	else if( strcmp( "rendermode", pszKey ) == 0 )
	{
		m_RenderMode = static_cast<RenderMode>( strtol( pszValue, nullptr, 10 ) );

		if( m_RenderMode < RenderMode::FIRST )
			m_RenderMode = RenderMode::FIRST;
		else if( m_RenderMode > RenderMode::LAST )
			m_RenderMode = RenderMode::LAST;

		return true;
	}
	else if( strcmp( "renderamt", pszKey ) == 0 )
	{
		m_flRenderAmount = static_cast<float>( clamp( 0.0, strtod( pszValue, nullptr ), 255.0 ) );
		return true;
	}
	else if( strcmp( "model", pszKey ) == 0 )
	{
		for( int iIndex = 0; iIndex < BSP::mod_numknown; ++iIndex )
		{
			if( !BSP::mod_known[ iIndex ].name[ 0 ] )
				break;

			if( strcmp( pszValue, BSP::mod_known[ iIndex ].name ) == 0 )
			{
				m_pModel = &BSP::mod_known[ iIndex ];
				break;
			}
		}

		return true;
	}

	return false;
}

void CBaseEntity::Spawn()
{
}