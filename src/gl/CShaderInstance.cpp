#include <cassert>
#include <cstdio>
#include <memory>

#include <glm/gtc/type_ptr.hpp>

#include "GLUtil.h"

#include "CBaseShader.h"

#include "CShaderInstance.h"

//Keep this in sync with the types.
static const size_t AttribSizes[ static_cast<size_t>( AttributeType::NUM_TYPES ) ] =
{
	sizeof( GLint ),		//INTEGER
	sizeof( GLfloat ),		//FLOAT
	sizeof( glm::vec2 ),	//VEC2
	sizeof( glm::vec3 ),	//VEC3
	sizeof( glm::vec4 ),	//VEC4
	sizeof( glm::mat4x4 ),	//MAT4X4
	0						//SAMPLER_TEXTURE
};

static const GLenum TypeToGLEnum[ static_cast<size_t>( AttributeType::NUM_TYPES ) ] =
{
	GL_INT,		//INTEGER
	GL_FLOAT,	//FLOAT
	GL_FLOAT,	//VEC2
	GL_FLOAT,	//VEC3
	GL_FLOAT,	//VEC4
	GL_FLOAT,	//MAT4X4
	GL_INT,		//SAMPLER_TEXTURE
};

static const GLint ElementCounts[ static_cast<size_t>( AttributeType::NUM_TYPES ) ] = 
{
	1,		//INTEGER
	1,		//FLOAT
	2,		//VEC2
	3,		//VEC3
	4,		//VEC4
	4 * 4,	//MAT4X4
	1,		//SAMPLER_TEXTURE
};

static const char* const TypeToString[ static_cast<size_t>( AttributeType::NUM_TYPES ) ] =
{
	"integer",		//INTEGER
	"float",		//FLOAT
	"vec2",			//VEC2
	"vec3",			//VEC3
	"vec4",			//VEC4
	"mat4x4",		//MAT4X4
	"sampler2D",	//SAMPLER_TEXTURE
};

CShaderInstance::CShaderInstance()
{
}

CShaderInstance::~CShaderInstance()
{
	delete[] m_pUniforms;
	delete[] m_pAttributes;

	if( IsValid() )
	{
		glDeleteProgram( m_Program );
		m_Program = 0;
	}
}

bool CShaderInstance::Initialize( CBaseShader* pShader )
{
	assert( pShader );
	assert( m_Program == 0 );

	m_pShader = pShader;

	const char* const pszName = pShader->GetName();

	std::unique_ptr<char[]> vertex( LoadShaderFile( pszName, SHADER_VERTEX_EXT ) );
	std::unique_ptr<char[]> frag( LoadShaderFile( pszName, SHADER_FRAG_EXT ) );

	if( !vertex || !frag )
		return false;

	GLuint vertexShader;
	GLuint fragShader;

	if( !CreateShader( pszName, vertex.get(), GL_VERTEX_SHADER, vertexShader ) )
		return false;

	if( !CreateShader( pszName, frag.get(), GL_FRAGMENT_SHADER, fragShader ) )
	{
		glDeleteShader( vertexShader );

		check_gl_error();

		return false;
	}

	m_Program = glCreateProgram();

	check_gl_error();

	glAttachShader( m_Program, vertexShader );

	check_gl_error();

	glAttachShader( m_Program, fragShader );

	check_gl_error();

	OnPreLink();

	//Link program
	glLinkProgram( m_Program );

	check_gl_error();

	//Mark shaders as deleted so they'll be freed when the program is deleted.
	glDeleteShader( fragShader );

	check_gl_error();

	glDeleteShader( vertexShader );

	check_gl_error();

	//Check for errors
	GLint programSuccess = GL_TRUE;
	glGetProgramiv( m_Program, GL_LINK_STATUS, &programSuccess );

	check_gl_error();

	if( programSuccess != GL_TRUE )
	{
		printf( "Error linking shader program \"%s\" (%d)!\n", pszName, m_Program );
		PrintProgramLog( m_Program );

		glDeleteProgram( m_Program );

		check_gl_error();

		m_Program = 0;

		return false;
	}

	if( !OnPostLink() )
	{
		printf( "Error post-linking shader program \"%s\" (%d)!\n", pszName, m_Program );
		glDeleteProgram( m_Program );

		check_gl_error();

		m_Program = 0;

		return false;
	}

	return true;
}

void CShaderInstance::Bind()
{
	assert( IsValid() );

	glUseProgram( m_Program );

	check_gl_error();
}

void CShaderInstance::Unbind()
{
	glUseProgram( 0 );

	check_gl_error();
}

void CShaderInstance::EnableVAA()
{
	for( size_t uiIndex = 0; uiIndex < m_uiNumAttributes; ++uiIndex )
	{
		glEnableVertexAttribArray( m_pAttributes[ uiIndex ] );

		check_gl_error();
	}
}

void CShaderInstance::DisableVAA()
{
	for( size_t uiIndex = 0; uiIndex < m_uiNumAttributes; ++uiIndex )
	{
		glDisableVertexAttribArray( m_pAttributes[ uiIndex ] );

		check_gl_error();
	}
}

void CShaderInstance::SetupParams( const glm::mat4x4& projection, const glm::mat4x4& view, const glm::mat4x4& model )
{
	glUniformMatrix4fv( m_MatProjUniform, 1, GL_FALSE, glm::value_ptr( projection ) );

	check_gl_error();

	glUniformMatrix4fv( m_MatViewUniform, 1, GL_FALSE, glm::value_ptr( view ) );

	check_gl_error();

	glUniformMatrix4fv( m_MatModelUniform, 1, GL_FALSE, glm::value_ptr( model ) );

	check_gl_error();

	//Set samplers.
	//TODO: apparently these can be set in the shader file itself. Consider replacing this with that.
	CBaseShaderAttribute* pUniform;

	GLint iSampler = 0;

	for( size_t uiIndex = 0; uiIndex < m_uiNumUniforms; ++uiIndex )
	{
		pUniform = m_pShader->GetUniform( uiIndex );

		if( pUniform->GetType() == AttributeType::SAMPLER_TEXTURE )
		{
			glUniform1i( m_pUniforms[ uiIndex ], iSampler++ );

			check_gl_error();
		}
	}
}

void CShaderInstance::Activate( const CBaseEntity* pEntity )
{
	m_pShader->Activate( this, pEntity );
}

void CShaderInstance::SetupVertexAttribs()
{
	CBaseShaderAttribute* pAttrib;

	size_t uiOffset = 0;

	//TODO: provide an option for normalized param.
	for( size_t uiIndex = 0; uiIndex < m_uiNumAttributes; ++uiIndex )
	{
		pAttrib = m_pShader->GetAttribute( uiIndex );

		glVertexAttribPointer( 
			m_pAttributes[ uiIndex ], 
			ElementCounts[ static_cast<size_t>( pAttrib->GetType() ) ], 
			TypeToGLEnum[ static_cast<size_t>( pAttrib->GetType() ) ], 
			GL_FALSE, 
			m_uiAttribSizeInBytes, reinterpret_cast<void*>( uiOffset ) );

		uiOffset += AttribSizes[ static_cast<size_t>( pAttrib->GetType() ) ];

		check_gl_error();
	}
}

void CShaderInstance::Draw( const size_t uiNumVerts )
{
	m_pShader->OnDraw( this, uiNumVerts );
}

void CShaderInstance::OnPreLink()
{
	const size_t uiCount = m_pShader->GetNumOutputs();

	CBaseShaderOutput* pOutput;

	for( size_t uiIndex = 0; uiIndex < uiCount; ++uiIndex )
	{
		pOutput = m_pShader->GetOutput( uiIndex );

		glBindFragDataLocation( m_Program, static_cast<GLuint>( uiIndex ), pOutput->GetName() );
	}
}

bool CShaderInstance::OnPostLink()
{
	m_uiNumAttributes = m_pShader->GetNumAttributes();

	CBaseShaderAttribute* pAttrib;

	m_pAttributes = new GLint[ m_uiNumAttributes ];

	bool bSuccess = true;

	for( size_t uiIndex = 0; uiIndex < m_uiNumAttributes; ++uiIndex )
	{
		pAttrib = m_pShader->GetAttribute( uiIndex );

		m_pAttributes[ uiIndex ] = glGetAttribLocation( m_Program, pAttrib->GetName() );

		if( m_pAttributes[ uiIndex ] == -1 )
		{
			printf( "Failed to find shader attribute \"%s %s\" (Index %u)\n", TypeToString[ static_cast<size_t>( pAttrib->GetType() ) ], pAttrib->GetName(), pAttrib->GetIndex() );

			bSuccess = false;
		}
	}

	struct MatData
	{
		GLint& index;
		const char* const pszName;
	};

	MatData mats[ 3 ] = 
	{
		{ m_MatProjUniform, "matProj" },
		{ m_MatViewUniform, "matView" },
		{ m_MatModelUniform, "matModel" }
	};

	for( auto& mat : mats )
	{
		mat.index = glGetUniformLocation( m_Program, mat.pszName );

		if( mat.index == -1 )
		{
			printf( "Failed to find shader uniform \"%s %s\"\n", TypeToString[ static_cast<size_t>( AttributeType::MAT4X4 ) ], mat.pszName );

			bSuccess = false;
		}
	}

	m_uiNumUniforms = m_pShader->GetNumUniforms();

	m_pUniforms = new GLint[ m_uiNumUniforms ];

	for( size_t uiIndex = 0; uiIndex < m_uiNumUniforms; ++uiIndex )
	{
		pAttrib = m_pShader->GetUniform( uiIndex );

		m_pUniforms[ uiIndex ] = glGetUniformLocation( m_Program, pAttrib->GetName() );

		if( m_pUniforms[ uiIndex ] == -1 )
		{
			printf( "Failed to find shader uniform \"%s %s\" (Index %u)\n", TypeToString[ static_cast<size_t>( pAttrib->GetType() ) ], pAttrib->GetName(), pAttrib->GetIndex() );

			bSuccess = false;
		}
	}

	if( bSuccess )
	{
		//Precalculate the per-vertex data size.
		m_uiAttribSizeInBytes = 0;

		CBaseShaderAttribute* pAttrib;

		for( size_t uiIndex = 0; uiIndex < m_uiNumAttributes; ++uiIndex )
		{
			pAttrib = m_pShader->GetAttribute( uiIndex );

			m_uiAttribSizeInBytes += AttribSizes[ static_cast<size_t>( pAttrib->GetType() ) ];
		}
	}

	return bSuccess;
}

char* CShaderInstance::LoadShaderFile( const char* const pszName, const char* const pszExt )
{
	assert( pszName );
	assert( pszExt );

	//TODO: use platform independent size - Solokiller
	char szBuffer[ 1024 ];

	const int iResult = snprintf( szBuffer, sizeof( szBuffer ), "%s%s%s", SHADER_BASE_DIR, pszName, pszExt );

	if( iResult < 0 || static_cast<size_t>( iResult ) >= sizeof( szBuffer ) )
		return nullptr;

	if( FILE* pFile = fopen( szBuffer, "rb" ) )
	{
		fseek( pFile, 0, SEEK_END );

		const auto size = ftell( pFile );

		fseek( pFile, 0, SEEK_SET );

		char* pszData = new char[ size + 1 ];

		const bool bSuccess = fread( pszData, size, 1, pFile ) == 1;

		fclose( pFile );

		if( bSuccess )
		{
			pszData[ size ] = '\0';
			return pszData;
		}

		delete[] pszData;
	}

	printf( "Failed to load shader file \"%s\"\n", szBuffer );

	return nullptr;
}

bool CShaderInstance::CreateShader( const char* const pszName, const char* const pszSource, const GLenum type, GLuint& shader )
{
	shader = glCreateShader( type );

	glShaderSource( shader, 1, &pszSource, nullptr );

	glCompileShader( shader );

	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &vShaderCompiled );

	if( vShaderCompiled == GL_TRUE )
		return true;

	//TODO: handle all shader types - Solokiller
	printf( "Failed to compile %s shader \"%s\"\n", type == GL_VERTEX_SHADER ? "vertex" : "fragment", pszName );

	PrintShaderLog( shader );

	glDeleteShader( shader );

	shader = 0;

	return false;
}

void CShaderInstance::PrintShaderLog( GLuint shader )
{
	//Make sure name is shader
	if( glIsShader( shader ) )
	{
		//Shader log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );

		//Allocate string
		char* infoLog = new char[ maxLength ];

		//Get info log
		glGetShaderInfoLog( shader, maxLength, &infoLogLength, infoLog );
		if( infoLogLength > 0 )
		{
			//Print Log
			printf( "%s\n", infoLog );
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		printf( "Name %d is not a shader\n", shader );
	}
}

void CShaderInstance::PrintProgramLog( GLuint program )
{
	//Make sure name is shader
	if( glIsProgram( program ) )
	{
		//Program log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &maxLength );

		//Allocate string
		char* infoLog = new char[ maxLength ];

		//Get info log
		glGetProgramInfoLog( program, maxLength, &infoLogLength, infoLog );
		if( infoLogLength > 0 )
		{
			//Print Log
			printf( "%s\n", infoLog );
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		printf( "Name %d is not a program\n", program );
	}
}