#ifndef GL_CLIGHTMAPPEDGENERICSHADER_H
#define GL_CLIGHTMAPPEDGENERICSHADER_H

#include "CBaseShader.h"

/**
*	Draws a lightmapped polygon.
*/
class CLightMappedGenericShader : public CBaseShader
{
public:
	using CBaseShader::CBaseShader;

	virtual void EnableVAA() override;

	virtual void DisableVAA() override;

	GLint GetVertexArrayAttrib() const { return m_VertexArrayAttrib; }

	GLint GetMatProjUniform() const { return m_MatProjUniform; }

	void SetupParams( const glm::mat4x4& projection, const glm::mat4x4& view, const glm::mat4x4& model ) override;

	void SetupVertexAttribs() override;

protected:
	virtual void OnPreLink() override;

	virtual bool OnPostLink() override;

private:
	GLint m_VertexArrayAttrib = -1;
	GLint m_TexCoordAttrib = -1;
	GLint m_LightmapCoordAttrib = -1;
	GLint m_MatProjUniform = -1;
	GLint m_MatViewUniform = -1;
	GLint m_MatModelUniform = -1;
};

#endif //GL_CLIGHTMAPPEDGENERICSHADER_H