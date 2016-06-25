#ifndef GL_CBASESHADER_H
#define GL_CBASESHADER_H

#include <gl/glew.h>

#include <glm/mat4x4.hpp>

#define SHADER_BASE_DIR "shaders/"
#define SHADER_VERTEX_EXT ".vtx"
#define SHADER_FRAG_EXT ".frag"

/**
*	Base class for shaders.
*/
class CBaseShader
{
public:
	/**
	*	Constructor.
	*	@param pszName Name of the shader. Also the name of the shader files.
	*/
	CBaseShader( const char* const pszName );

	/**
	*	Destructor.
	*/
	~CBaseShader();

	/**
	*	@return First shader in the global list.
	*/
	static CBaseShader* GetHead() { return m_pHead; }

	/**
	*	@return Next shader in the global list.
	*/
	CBaseShader* GetNext() const { return m_pNext; }

	/**
	*	@return Name of this shader.
	*/
	const char* GetName() const { return m_pszName; }

	/**
	*	@return This shader's program ID.
	*/
	GLuint GetProgramID() const { return m_Program; }

	/**
	*	@return Whether this shader is valid.
	*/
	bool IsValid() const { return m_Program != 0; }

	/**
	*	Loads this shader.
	*/
	bool Load();

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
	virtual void EnableVAA() {}

	/**
	*	Disable Vertex Attribute Arrays.
	*/
	virtual void DisableVAA() {}

	/**
	*	Set up parameters for drawing.
	*/
	virtual void SetupParams( const glm::mat4x4& projection, const glm::mat4x4& view, const glm::mat4x4& model ) {}

	/**
	*	Set up vertex attributes for drawing.
	*/
	virtual void SetupVertexAttribs() {}

protected:
	/**
	*	Called after compilation, before linking.
	*/
	virtual void OnPreLink() {}

	/**
	*	Called after linking has succeeded.
	*	@return true if the shader is valid, false otherwise.
	*/
	virtual bool OnPostLink() { return true; }

private:
	/**
	*	Loads the given shader and initializes this shader.
	*/
	bool Load( const char* const pszName );

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
	static CBaseShader* m_pHead;
	CBaseShader* m_pNext;

	const char* const m_pszName;

	GLuint m_Program = 0;

private:
	CBaseShader( const CBaseShader& ) = delete;
	CBaseShader& operator=( const CBaseShader& ) = delete;
};

#endif //GL_CBASESHADER_H