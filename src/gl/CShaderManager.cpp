#include <cassert>
#include <cstdio>

#include "GLUtil.h"

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

void CShaderManager::ActivateShader( CShaderInstance* pShader, const glm::mat4x4& projection, const glm::mat4x4& view, const glm::mat4x4& model, const CBaseEntity* pEntity )
{
	assert( pShader );

	if( !pShader )
		return;

	if( m_pActiveShader != pShader )
	{
		if( m_pActiveShader )
			DeactivateActiveShader();

		m_pActiveShader = pShader;

		m_pActiveShader->Bind();

		check_gl_error();

		m_pActiveShader->EnableVAA();

		check_gl_error();

		m_pActiveShader->SetupParams( projection, view, model );

		check_gl_error();
	}

	m_pActiveShader->Activate( pEntity );
}

void CShaderManager::DeactivateActiveShader()
{
	if( !m_pActiveShader )
		return;

	m_pActiveShader->DisableVAA();

	CShaderInstance::Unbind();

	m_pActiveShader = nullptr;
}