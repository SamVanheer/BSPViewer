#include <cassert>
#include <cstdio>

#include "CBaseEntity.h"

#include "CEntityList.h"

CEntityList g_EntList;

CBaseEntity* CEntityList::GetFirstEntity()
{
	return GetNextEntity( nullptr );
}

CBaseEntity* CEntityList::GetNextEntity( CBaseEntity* pStart )
{
	size_t uiIndex = pStart ? pStart->GetEntIndex() + 1 : 0;

	for( ; uiIndex < m_Entities.size(); ++uiIndex )
	{
		if( m_Entities[ uiIndex ] )
			return m_Entities[ uiIndex ];
	}

	return nullptr;
}

CBaseEntity* CEntityList::Create( const char* const pszClassName )
{
	assert( pszClassName );

	size_t uiIndex;

	for( uiIndex = 0; uiIndex < m_Entities.size(); ++uiIndex )
	{
		if( !m_Entities[ uiIndex ] )
			break;
	}

	if( uiIndex == m_Entities.size() )
		m_Entities.push_back( nullptr );

	CBaseEntity* pEntity = new CBaseEntity();

	pEntity->Construct( pszClassName, uiIndex );

	pEntity->OnCreate();

	m_Entities[ uiIndex ] = pEntity;

	++m_uiCount;

	return pEntity;
}

void CEntityList::Destroy( CBaseEntity* pEntity )
{
	if( !pEntity )
		return;

	const size_t uiIndex = pEntity->GetEntIndex();

	if( m_Entities.size() < uiIndex || m_Entities[ uiIndex ] != pEntity )
	{
		printf( "CEntityList::Destroy: Entity index is invalid!\n" );
		return;
	}

	pEntity->OnDestroy();

	delete pEntity;

	m_Entities[ uiIndex ] = nullptr;

	--m_uiCount;
}

void CEntityList::Clear()
{
	for( auto pEntity : m_Entities )
	{
		Destroy( pEntity );
	}

	m_Entities.clear();
	m_Entities.shrink_to_fit();
}