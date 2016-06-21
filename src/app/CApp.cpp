#include <cstdio>

#include <SDL.h>

#include <GL\glew.h>

#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "ui/CWindow.h"
#include "ui/CWindowArgs.h"
#include "ui/CWindowManager.h"

#include "gl/CShaderManager.h"

#include "gl/CBaseShader.h"

#include "bsp/BSPIO.h"

#include "CApp.h"

int CApp::Run( int iArgc, char* pszArgV[] )
{
	bool bSuccess = Initialize();

	if( bSuccess )
	{
		printf( "CApp::Initialize succeeded\n" );

		//Too big to put onto the stack - Solokiller
		CBSPFile* pFile = new CBSPFile;

		if( LoadBSPFile( "external/datacore.bsp", *pFile ) )
		{
			printf( "loaded BSP file\n" );
		}
		else
		{
			printf( "Failed to load BSP file\n" );
		}

		delete pFile;

		bSuccess = RunApp();
	}

	Shutdown();

	getchar();

	return bSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}

bool CApp::Initialize()
{
	bool bSuccess = g_WindowManager.Initialize();

	if( bSuccess )
	{
		m_pWindow = g_WindowManager.CreateWindow( CWindowArgs().Title( "BSP Viewer" ).Center().Size( 640, 480 ).Show().AddFlags( SDL_WINDOW_RESIZABLE ) );

		bSuccess = m_pWindow != nullptr;
	}

	if( bSuccess )
	{
		g_WindowManager.MakeGLCurrent( m_pWindow );

		bSuccess = g_ShaderManager.LoadShaders();
	}

	return bSuccess;
}

void CApp::Shutdown()
{
	g_WindowManager.DestroyWindow( m_pWindow );
	m_pWindow = nullptr;

	g_WindowManager.Shutdown();
}

bool CApp::RunApp()
{
	bool bSuccess = true;

	//VBO data
	GLfloat vertexData[] =
	{
		100, 100,
		200, 100,
		200, 200,
		100, 200
	};

	//IBO data
	GLuint indexData[] = { 0, 1, 2, 3 };

	//Create VBO
	glGenBuffers( 1, &m_VBO );
	glBindBuffer( GL_ARRAY_BUFFER, m_VBO );
	glBufferData( GL_ARRAY_BUFFER, 2 * 4 * sizeof( GLfloat ), vertexData, GL_STATIC_DRAW );

	//Create IBO
	glGenBuffers( 1, &m_IBO );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_IBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof( GLuint ), indexData, GL_STATIC_DRAW );

	//Main loop flag
	bool quit = false;

	//Event handler
	SDL_Event e;

	//Enable text input
	//SDL_StartTextInput();

	//TODO: shaders should be managed in a different way. Eventually this will be solved - Solokiller
	m_pPolygonShader = g_ShaderManager.GetShader( "Polygon" );

	//While application is running
	while( !quit )
	{
		//Handle events on queue
		while( SDL_PollEvent( &e ) != 0 )
		{
			if( e.type == SDL_WINDOWEVENT )
			{
				//Close if the main window receives a close request.
				if( e.window.event == SDL_WINDOWEVENT_CLOSE )
				{
					if( m_pWindow->GetWindowId() == e.window.windowID )
					{
						quit = true;
					}
				}
			}

			//User requests quit
			if( e.type == SDL_QUIT )
			{
				quit = true;
			}
		}

		Render();
	}

	glDeleteBuffers( 1, &m_VBO );
	glDeleteBuffers( 1, &m_IBO );

	return true;
}

void CApp::Render()
{
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

	//Render quad
	//Clear color buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	int width, height;

	m_pWindow->GetSize( width, height );

	glViewport( 0, 0, width, height );

	glDisable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );

	const double flAspect = static_cast<float>( width ) / static_cast<float>( height );

	auto projection = glm::ortho( 0.0f, static_cast<float>( width ), static_cast<float>( height ), 0.0f, 1.0f, -1.0f );

	glm::mat4x4 view = glm::mat4x4();

	glm::mat4x4 model = glm::mat4x4();

	//Render quad
	//Bind program
	m_pPolygonShader->Bind();

	//Enable vertex position
	m_pPolygonShader->EnableVAA();

	//Set vertex data
	glBindBuffer( GL_ARRAY_BUFFER, m_VBO );
	m_pPolygonShader->SetupParams( projection, view, model );

	//Set index data and render
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_IBO );
	glDrawElements( GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL );

	//Disable vertex position
	m_pPolygonShader->DisableVAA();

	//Unbind program
	CBaseShader::Unbind();

	//Update screen
	m_pWindow->SwapGLBuffers();
}