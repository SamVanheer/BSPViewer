#ifndef GL_CSHADERINSTANCE_H
#define GL_CSHADERINSTANCE_H

#include <glm/mat4x4.hpp>

#include <gl/glew.h>

class CBaseShader;
class CBaseEntity;

class CShaderInstance final
{
public:
	CShaderInstance();
	~CShaderInstance();

	/**
	*	@return Whether this shader is valid.
	*/
	bool IsValid() const { return m_Program != 0; }

	bool Initialize( CBaseShader* pShader );

	/**
	*	Binds this shader.
	*/
	void Bind();

	/**
	*	Unbinds the current shader.
	*/
	static void Unbind();

	/**
	*	Enable Vertex Attribute Arrays.
	*/
	void EnableVAA();

	/**
	*	Disable Vertex Attribute Arrays.
	*/
	void DisableVAA();

	/**
	*	Set up parameters for drawing.
	*/
	void SetupParams( const glm::mat4x4& projection, const glm::mat4x4& view, const glm::mat4x4& model );

	/**
	*	Activates the shader for the given entity.
	*/
	void Activate( const CBaseEntity* pEntity );

	/**
	*	Set up vertex attributes for drawing.
	*/
	void SetupVertexAttribs();

	/**
	*	Draw current data.
	*/
	void Draw( const size_t uiNumVerts );

	const GLint* GetAttributes() const { return m_pAttributes; }

	const GLint* GetUniforms() const { return m_pUniforms; }

private:
	/**
	*	Called after compilation, before linking.
	*/
	void OnPreLink();

	/**
	*	Called after linking has succeeded.
	*	@return true if the shader is valid, false otherwise.
	*/
	bool OnPostLink();

	/**
	*	Loads a shader file.
	*	@param pszName Shader name.
	*	@param pszExt File extension.
	*/
	static char* LoadShaderFile( const char* const pszName, const char* const pszExt );

	/**
	*	Creates a shader object.
	*	@param pszName Shader name.
	*	@param pszSource Shader source code.
	*	@param type Shader type.
	*	@param[ out ] shader Contains the shader ID if the operation was successful.
	*	@return true on success, false otherwise.
	*/
	static bool CreateShader( const char* const pszName, const char* const pszSource, const GLenum type, GLuint& shader );

	/**
	*	Prints the shader compilation log for the given shader.
	*/
	static void PrintShaderLog( GLuint shader );

	/**
	*	Prints the program linking log for the given program.
	*/
	static void PrintProgramLog( GLuint program );

private:
	CBaseShader* m_pShader = nullptr;

	GLuint m_Program = 0;

	size_t m_uiNumAttributes = 0;
	GLint* m_pAttributes = nullptr;

	GLint m_MatProjUniform = -1;
	GLint m_MatViewUniform = -1;
	GLint m_MatModelUniform = -1;

	size_t m_uiNumUniforms = 0;
	GLint* m_pUniforms = nullptr;

	size_t m_uiAttribSizeInBytes = 0;
};

#endif //GL_CSHADERINSTANCE_H