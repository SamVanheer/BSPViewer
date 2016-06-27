#ifndef ENTITY_CENTITYLIST_H
#define ENTITY_CENTITYLIST_H

#include <vector>

class CBaseEntity;

/**
*	Naïve entity list.
*/
class CEntityList final
{
private:
	typedef std::vector<CBaseEntity*> Entities_t;

public:
	CEntityList() = default;
	~CEntityList() = default;

	size_t GetEntityCount() const { return m_uiCount; }

	CBaseEntity* GetFirstEntity();

	CBaseEntity* GetNextEntity( CBaseEntity* pStart );

	CBaseEntity* Create( const char* const pszClassName );

	void Destroy( CBaseEntity* pEntity );

	void Clear();

private:
	Entities_t m_Entities;

	size_t m_uiCount = 0;

private:
	CEntityList( const CEntityList& ) = delete;
	CEntityList& operator=( const CEntityList& ) = delete;
};

extern CEntityList g_EntList;

#endif //ENTITY_CENTITYLIST_H