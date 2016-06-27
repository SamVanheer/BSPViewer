#ifndef ENTITY_CBASEENTITY_H
#define ENTITY_CBASEENTITY_H

#include <string>

#include "utility/Mathlib.h"

#undef GetClassName

struct bmodel_t;

enum class RenderMode
{
	FIRST		= 0,

	NORMAL		= FIRST,
	COLOR,
	TEXTURE,
	GLOW,
	SOLID,
	ADDITIVE,

	LAST		= ADDITIVE,

	NUM
};

class CBaseEntity
{
public:
	static const size_t INVALID_ENT_INDEX = -1;

public:
	CBaseEntity() = default;
	~CBaseEntity() = default;

	void Construct( const char* const pszClassName, const size_t uiEntIndex );

	virtual void OnCreate();

	virtual void OnDestroy();

	virtual bool KeyValue( const char* pszKey, const char* pszValue );

	virtual void Spawn();

	const char* GetClassName() const { return m_szClassName.c_str(); }

	size_t GetEntIndex() const { return m_uiEntIndex; }

	bmodel_t* GetBrushModel() const { return m_pModel; }

	const Vector& GetOrigin() const { return m_vecOrigin; }

	Vector& GetMutableOrigin() { return m_vecOrigin; }

	const Vector& GetAngles() const { return m_vecAngles; }

	Vector& GetMutableAngles() { return m_vecAngles; }

	RenderMode GetRenderMode() const { return m_RenderMode; }

	float GetRenderAmount() const { return m_flRenderAmount; }

private:
	std::string m_szClassName;

	size_t m_uiEntIndex = INVALID_ENT_INDEX;

	bmodel_t* m_pModel = nullptr;

	Vector m_vecOrigin;
	Vector m_vecAngles;

	RenderMode m_RenderMode = RenderMode::NORMAL;

	float m_flRenderAmount = 0;

private:
	CBaseEntity( const CBaseEntity& ) = delete;
	CBaseEntity& operator=( const CBaseEntity& ) = delete;
};

#endif //ENTITY_CBASEENTITY_H