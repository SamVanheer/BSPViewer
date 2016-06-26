#include <cassert>
#include <cstdio>

#include "CBaseShader.h"
#include "CShaderInstance.h"

#include "CShaderManager.h"

CShaderManager g_ShaderManager;

bool CShaderManager::LoadShaders()
{
	for( auto pShader = CBaseShader::GetHead(); pShader; pShader = pShader->GetNext() )
	{
		if( !AddShader( pShader ) )
			return false;
	}

	return true;
}

CShaderInstance* CShaderManager::GetShader( const char* const pszName )
{
	assert( pszName );

	auto it = m_Shaders.find( pszName );

	if( it != m_Shaders.end() )
		return it->second;

	return nullptr;
}

bool CShaderManager::AddShader( CBaseShader* pShader )
{
	assert( pShader );

	auto it = m_Shaders.find( pShader->GetName() );

	if( it != m_Shaders.end() )
	{
		printf( "CShaderManager::AddShader: Duplicate shader \"%s\"!\n", pShader->GetName() );
		return false;
	}

	CShaderInstance* pInstance = new CShaderInstance();

	if( !pInstance->Initialize( pShader ) )
	{
		printf( "CShaderManager::AddShader: Shader \"%s\" failed to load!\n", pShader->GetName() );

		delete pInstance;

		return false;
	}

	m_Shaders.insert( std::make_pair( pShader->GetName(), pInstance ) );

	return true;
}