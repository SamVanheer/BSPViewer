#ifndef GL_CPOLYGONSHADER_H
#define GL_CPOLYGONSHADER_H

#include "CBaseShader.h"

/**
*	Draws a polygon.
*/
class CPolygonShader : public CBaseShader
{
public:
	using CBaseShader::CBaseShader;

	virtual void EnableVAA() override;

	virtual void DisableVAA() override;

	GLint GetVertexArrayAttrib() const { return m_VertexArrayAttrib; }

	GLint GetMatProjUniform() const { return m_MatProjUniform; }

	void SetupParams( const glm::mat4x4& projection, const glm::mat4x4& view, const glm::mat4x4& model ) override;

protected:
	virtual void OnPreLink() override;

	virtual bool OnPostLink() override;

private:
	GLint m_VertexArrayAttrib = -1;
	GLint m_MatProjUniform = -1;
};

#endif //GL_CPOLYGONSHADER_H