#include <glm/gtc/type_ptr.hpp>

#include "GLUtil.h"

#include "CLightMappedGenericShader.h"

static CLightMappedGenericShader g_LightMappedGeneircShader( "LightMappedGeneric" );

void CLightMappedGenericShader::EnableVAA()
{
	glEnableVertexAttribArray( m_VertexArrayAttrib );
	glEnableVertexAttribArray( m_TexCoordAttrib );
}

void CLightMappedGenericShader::DisableVAA()
{
	glDisableVertexAttribArray( m_TexCoordAttrib );
	glDisableVertexAttribArray( m_VertexArrayAttrib );
}

void CLightMappedGenericShader::SetupParams( const glm::mat4x4& projection, const glm::mat4x4& view, const glm::mat4x4& model )
{
	glVertexAttribPointer( GetVertexArrayAttrib(), 3, GL_FLOAT, GL_FALSE, 7 * sizeof( GLfloat ), NULL );

	check_gl_error();

	glVertexAttribPointer( m_TexCoordAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof( GLfloat ), reinterpret_cast<void*>( 3 * sizeof( GLfloat ) ) );
	
	check_gl_error();
	
	glUniformMatrix4fv( GetMatProjUniform(), 1, GL_FALSE, glm::value_ptr( projection ) );

	check_gl_error();

	glUniformMatrix4fv( m_MatViewUniform, 1, GL_FALSE, glm::value_ptr( view ) );

	check_gl_error();

	glUniformMatrix4fv( m_MatModelUniform, 1, GL_FALSE, glm::value_ptr( model ) );
}

void CLightMappedGenericShader::OnPreLink()
{
	glBindFragDataLocation( GetProgramID(), 0, "outColor" );
}

bool CLightMappedGenericShader::OnPostLink()
{
	m_VertexArrayAttrib = glGetAttribLocation( GetProgramID(), "vecPosition" );
	m_TexCoordAttrib = glGetAttribLocation( GetProgramID(), "vecTexCoord" );
	m_LightmapCoordAttrib = glGetAttribLocation( GetProgramID(), "vecLightmapCoord" );
	m_MatProjUniform = glGetUniformLocation( GetProgramID(), "matProj" );
	m_MatViewUniform = glGetUniformLocation( GetProgramID(), "matView" );
	m_MatModelUniform = glGetUniformLocation( GetProgramID(), "matModel" );

	return m_VertexArrayAttrib != -1 && m_TexCoordAttrib !=-1 /*&& m_LightmapCoordAttrib != -1*/ && m_MatProjUniform != -1 && m_MatViewUniform != -1 && m_MatModelUniform != -1;
}