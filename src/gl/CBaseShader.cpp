#include <cassert>
#include <cstdio>
#include <cstring>
#include <memory>

#include "CBaseShader.h"

CBaseShader* CBaseShader::m_pHead = nullptr;

CBaseShader::CBaseShader( const char* const pszName )
	: m_pszName( pszName )
{
	assert( pszName );

	m_pNext = m_pHead;
	m_pHead = this;
}

CBaseShader::~CBaseShader()
{
	if( IsValid() )
	{
		glDeleteProgram( m_Program );
		m_Program = 0;
	}
}

bool CBaseShader::Load()
{
	return Load( m_pszName );
}

void CBaseShader::Bind()
{
	assert( IsValid() );

	glUseProgram( m_Program );
}

void CBaseShader::Unbind()
{
	glUseProgram( 0 );
}

bool CBaseShader::Load( const char* const pszName )
{
	assert( m_Program == 0 );
	assert( pszName );

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
		return false;
	}

	m_Program = glCreateProgram();

	glAttachShader( m_Program, vertexShader );
	glAttachShader( m_Program, fragShader );

	OnPreLink();

	//Link program
	glLinkProgram( m_Program );

	//Mark shaders as deleted so they'll be freed when the program is deleted.
	glDeleteShader( fragShader );
	glDeleteShader( vertexShader );

	//Check for errors
	GLint programSuccess = GL_TRUE;
	glGetProgramiv( m_Program, GL_LINK_STATUS, &programSuccess );

	if( programSuccess != GL_TRUE )
	{
		printf( "Error linking shader program \"%s\" (%d)!\n", pszName, m_Program );
		PrintProgramLog( m_Program );

		glDeleteProgram( m_Program );
		m_Program = 0;

		return false;
	}

	if( !OnPostLink() )
	{
		printf( "Error post-linking shader program \"%s\" (%d)!\n", pszName, m_Program );
		glDeleteProgram( m_Program );
		m_Program = 0;

		return false;
	}

	return true;
}

char* CBaseShader::LoadShaderFile( const char* const pszName, const char* const pszExt )
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

bool CBaseShader::CreateShader( const char* const pszName, const char* const pszSource, const GLenum type, GLuint& shader )
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

void CBaseShader::PrintShaderLog( GLuint shader )
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

void CBaseShader::PrintProgramLog( GLuint program )
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