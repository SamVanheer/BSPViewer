#include <cstdio>
#include <cstring>

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

#include "gl/GLUtil.h"

#include "bsp/BSPIO.h"
#include "bsp/BSPRenderIO.h"

#include "wad/WadIO.h"
#include "wad/CWadFile.h"
#include "wad/CWadManager.h"

#include "gl/GLMiptex.h"

#include "CApp.h"

const float CApp::ROTATE_SPEED = 120.0f;
const float CApp::MOVE_SPEED = 100.0f;

int CApp::Run( int iArgc, char* pszArgV[] )
{
	bool bSuccess = Initialize();

	if( bSuccess )
	{
		printf( "CApp::Initialize succeeded\n" );

		g_WadManager.SetBasePath( "external" );

		{
			auto header = LoadBSPFile( "external/boot_camp.bsp" );

			memset( &m_Model, 0, sizeof( bmodel_t ) );

			strcpy( m_Model.name, "external/test.bsp" );

			bSuccess = BSP::LoadBrushModel( &m_Model, header.get() );

			if( bSuccess )
			{
				printf( "Loaded BSP\n" );
			}
			else
			{
				printf( "Couldn't load BSP\n" );
			}
		}

		check_gl_error();

		if( bSuccess )
		{
			m_Camera.RotateYaw( -90.0f );

			printf( "Ready to begin\n" );
			getchar();

			bSuccess = RunApp();
		}

		BSP::FreeModel( &m_Model );
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
		m_pWindow = g_WindowManager.CreateWindow( CWindowArgs().Title( "BSP Viewer" ).Center().Size( 1024, 768 ).Show().AddFlags( SDL_WINDOW_RESIZABLE ) );

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

	//Main loop flag
	bool quit = false;

	//Event handler
	SDL_Event e;

	//Enable text input
	//SDL_StartTextInput();

	//TODO: shaders should be managed in a different way. Eventually this will be solved - Solokiller
	m_pPolygonShader = g_ShaderManager.GetShader( "Polygon" );

	m_pLightmapShader = g_ShaderManager.GetShader( "LightMappedGeneric" );

	check_gl_error();

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

			Event( e );
		}

		std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() );

		const auto diff = ( now - m_LastTick ).count();

		//Frame limit
		if( diff >= ( 1 / 60.0f ) )
		{
			m_flDeltaTime = diff / 1000.0f;

			printf( "delta: %f\n", m_flDeltaTime );

			if( m_flDeltaTime > 1.0f )
				m_flDeltaTime = 0.0f;

			printf( "delta: %f\n", m_flDeltaTime );

			m_LastTick = now;

			if( m_flYawVel )
				m_Camera.RotateYaw( m_flDeltaTime * m_flYawVel );

			if( m_flPitchVel )
				m_Camera.RotatePitch( m_flDeltaTime * m_flPitchVel );

			Render();
		}
	}

	return true;
}

void CApp::Render()
{
	check_gl_error();

	//Depth testing prevents objects that are further away from drawing on top of nearer objects
	glEnable( GL_DEPTH_TEST );

	check_gl_error();

	//Cull back faces
	glEnable( GL_CULL_FACE );

	check_gl_error();

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	check_gl_error();

	//We use clockwise order for triangle vertices
	glFrontFace( GL_CW );

	check_gl_error();

	int width, height;

	m_pWindow->GetSize( width, height );

	glViewport( 0, 0, width, height );

	const float flAspect = static_cast<float>( width ) / static_cast<float>( height );

	auto projection = glm::ortho( 0.0f, static_cast<float>( width ), static_cast<float>( height ), 0.0f, 1.0f, -1.0f );

	projection = glm::perspective( glm::radians( 75.0f ), flAspect, 0.1f, 10000.0f );

	glm::mat4x4 view;
	
	view = glm::mat4x4(
		0, -1, 0, 0,
		0, 0, 1, 0,
		-1, 0, 0, 0,
		0, 0, 0, 1 );

	view = m_Camera.GetViewMatrix();

	glm::mat4x4 model = glm::mat4x4();

	m_pLightmapShader->Bind();

	check_gl_error();

	m_pLightmapShader->EnableVAA();

	check_gl_error();

	m_pLightmapShader->SetupParams( projection, view, model );

	check_gl_error();

	std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() );

	size_t uiCount = 0;

	size_t uiTriangles = 0;

	double flTotal = 0;

	//TODO: all models should be in the same list.
	RenderModel( m_Model, uiCount, uiTriangles, flTotal );

	for( int iIndex = 0; iIndex < BSP::mod_numknown; ++iIndex )
	{
		RenderModel( BSP::mod_known[ iIndex ], uiCount, uiTriangles, flTotal );
	}

	std::chrono::milliseconds now2 = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() );

	printf( "Time spent rendering frame (%u polygons, %u triangles, average (msec): %f): %f\n", uiCount, uiTriangles, flTotal / uiCount, ( now2 - now ).count() / 1000.0f );

	m_pLightmapShader->DisableVAA();

	//Unbind program
	CBaseShader::Unbind();

	check_gl_error();

	//Update screen
	m_pWindow->SwapGLBuffers();

	check_gl_error();
}

void CApp::RenderModel( bmodel_t& model, size_t& uiCount, size_t& uiTriangles, double& flTotal )
{
	msurface_t* pSurface = model.surfaces + model.firstmodelsurface;

	for( int iIndex = 0; iIndex < model.nummodelsurfaces; ++iIndex, ++pSurface )
	{
		glActiveTexture( GL_TEXTURE0 + 1 );

		check_gl_error();

		//Skies will have no texture here.
		glBindTexture( GL_TEXTURE_2D, pSurface->lightmaptexturenum );

		check_gl_error();

		glActiveTexture( GL_TEXTURE0 + 0 );

		check_gl_error();

		for( glpoly_t* pPoly = pSurface->polys; pPoly; pPoly = pPoly->chain )
		{
			if( pSurface->texinfo->texture )
				glBindTexture( GL_TEXTURE_2D, pSurface->texinfo->texture->gl_texturenum );
			else
				glBindTexture( GL_TEXTURE_2D, 0 );

			check_gl_error();

			for( glpoly_t* pPoly2 = pPoly; pPoly2; pPoly2 = pPoly2->next )
			{
				std::chrono::milliseconds start = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() );

				glBindBuffer( GL_ARRAY_BUFFER, pPoly2->VBO );

				m_pLightmapShader->SetupVertexAttribs();

				check_gl_error();

				glDrawArrays( GL_POLYGON, 0, pPoly2->numverts );

				check_gl_error();

				++uiCount;

				uiTriangles += pPoly2->numverts - 2;

				std::chrono::milliseconds end = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() );

				flTotal += ( end - start ).count();
			}
		}
	}
}

void CApp::Event( const SDL_Event& event )
{
	switch( event.type )
	{
	case SDL_KEYDOWN:
	case SDL_KEYUP:				KeyEvent( event.key ); break;

	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:		MouseButtonEvent( event.button ); break;

	case SDL_MOUSEMOTION:		MouseMotionEvent( event.motion ); break;

	case SDL_MOUSEWHEEL:		MouseWheelEvent( event.wheel ); break;

	default: break;
	}
}

void CApp::KeyEvent( const SDL_KeyboardEvent& event )
{
	switch( event.type )
	{
	case SDL_KEYDOWN:
		switch( event.keysym.sym )
		{
		case SDLK_LEFT:		m_flYawVel = -ROTATE_SPEED; break;
		case SDLK_RIGHT:	m_flYawVel = ROTATE_SPEED; break;
		case SDLK_UP:		m_flPitchVel = -ROTATE_SPEED; break;
		case SDLK_DOWN:		m_flPitchVel = ROTATE_SPEED; break;
		default: break;
		}
		break;

	case SDL_KEYUP:
		switch( event.keysym.sym )
		{
		case SDLK_LEFT:		m_flYawVel = 0; break;
		case SDLK_RIGHT:	m_flYawVel = 0; break;
		case SDLK_UP:		m_flPitchVel = 0; break;
		case SDLK_DOWN:		m_flPitchVel = 0; break;
		default: break;
		}
		break;
	}
}

void CApp::MouseButtonEvent( const SDL_MouseButtonEvent& event )
{
}

void CApp::MouseMotionEvent( const SDL_MouseMotionEvent& event )
{
}

void CApp::MouseWheelEvent( const SDL_MouseWheelEvent& event )
{
	const glm::vec3 dir = m_Camera.GetDirection();

	m_Camera.GetPosition() += dir * ( MOVE_SPEED * event.y );
}