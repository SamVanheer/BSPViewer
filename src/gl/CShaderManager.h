#ifndef GL_CSHADERMANAGER_H
#define GL_CSHADERMANAGER_H

#include <string>
#include <unordered_map>

#include <glm/mat4x4.hpp>

#include "common/StringUtils.h"

class CBaseShader;
class CShaderInstance;
class CBaseEntity;

class CShaderManager final
{
private:

	typedef std::unordered_map<const char*, CShaderInstance*, RawCharHash, RawCharEqualTo> Shaders_t;

public:
	CShaderManager() = default;
	~CShaderManager() = default;

	bool LoadShaders();

	CShaderInstance* GetShader( const char* const pszName );

	CShaderInstance* GetActiveShader() const { return m_pActiveShader; }

	//TODO: the matrices should come from somewhere else.
	//TODO: shouldn't be directly referencing entities.
	void ActivateShader( CShaderInstance* pShader, const glm::mat4x4& projection, const glm::mat4x4& view, const glm::mat4x4& model, const CBaseEntity* pEntity );

	void DeactivateActiveShader();

private:
	bool AddShader( CBaseShader* pShader );

private:
	Shaders_t m_Shaders;

	CShaderInstance* m_pActiveShader = nullptr;

private:
	CShaderManager( const CShaderManager& ) = delete;
	CShaderManager& operator=( const CShaderManager& ) = delete;
};

extern CShaderManager g_ShaderManager;

#endif //GL_CSHADERMANAGER_H