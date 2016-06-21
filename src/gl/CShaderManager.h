#ifndef GL_CSHADERMANAGER_H
#define GL_CSHADERMANAGER_H

#include <string>
#include <unordered_map>

#include "common/StringUtils.h"

class CBaseShader;

class CShaderManager final
{
private:
	typedef std::unordered_map<const char*, CBaseShader*, RawCharHash, RawCharEqualTo> Shaders_t;

public:
	CShaderManager() = default;
	~CShaderManager() = default;

	bool LoadShaders();

	CBaseShader* GetShader( const char* const pszName );

private:
	bool AddShader( CBaseShader* pShader );

private:
	Shaders_t m_Shaders;

private:
	CShaderManager( const CShaderManager& ) = delete;
	CShaderManager& operator=( const CShaderManager& ) = delete;
};

extern CShaderManager g_ShaderManager;

#endif //GL_CSHADERMANAGER_H