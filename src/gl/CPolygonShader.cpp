#include <glm/gtc/type_ptr.hpp>

#include "CPolygonShader.h"

static CPolygonShader g_PolygonShader( "Polygon" );

void CPolygonShader::EnableVAA()
{
	glEnableVertexAttribArray( m_VertexArrayAttrib );
}

void CPolygonShader::DisableVAA()
{
	glDisableVertexAttribArray( m_VertexArrayAttrib );
}

void CPolygonShader::SetupParams( const glm::mat4x4& projection, const glm::mat4x4& view, const glm::mat4x4& model )
{
	glVertexAttribPointer( GetVertexArrayAttrib(), 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), NULL );
	glUniformMatrix4fv( GetMatProjUniform(), 1, GL_FALSE, glm::value_ptr( projection ) );
}

void CPolygonShader::OnPreLink()
{
	glBindFragDataLocation( GetProgramID(), 0, "outColor" );
}

bool CPolygonShader::OnPostLink()
{
	m_VertexArrayAttrib = glGetAttribLocation( GetProgramID(), "LVertexPos2D" );
	m_MatProjUniform = glGetUniformLocation( GetProgramID(), "matProj" );

	return m_VertexArrayAttrib != -1 && m_MatProjUniform != -1;
}