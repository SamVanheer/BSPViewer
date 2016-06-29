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

#include "gl/CShaderInstance.h"

#include "gl/GLUtil.h"

#include "bsp/BSPIO.h"
#include "bsp/BSPRenderIO.h"

#include "wad/WadIO.h"
#include "wad/CWadFile.h"
#include "wad/CWadManager.h"

#include "gl/GLMiptex.h"

#include "entity/CEntityList.h"
#include "entity/CBaseEntity.h"

#include "utility/Tokenization.h"

#include "CApp.h"

bool ED_FindClassName( char* data )
{
	char keyname[ 256 ];

	// go through all the dictionary pairs
	while( 1 )
	{
		// parse key
		data = COM_Parse( data );
		if( com_token[ 0 ] == '}' )
			break;
		if( !data )
		{
			printf( "ED_ParseEntity: EOF without closing brace\n" );
			return false;
		}

		strcpy( keyname, com_token );

		// another hack to fix heynames with trailing spaces
		size_t n = strlen( keyname );
		while( n && keyname[ n - 1 ] == ' ' )
		{
			keyname[ n - 1 ] = 0;
			n--;
		}

		// parse value	
		data = COM_Parse( data );
		if( !data )
		{
			printf( "ED_ParseEntity: EOF without closing brace\n" );
			return false;
		}

		if( com_token[ 0 ] == '}' )
		{
			printf( "ED_ParseEntity: closing brace without data\n" );
			return false;
		}

		if( strcmp( keyname, "classname" ) == 0 )
			return true;
	}

	return false;
}

/*
====================
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
Used for initial level load and for savegames.
====================
*/
bool ED_ParseEdict( char *data, char*& pszOut, CBaseEntity*& pEnt )
{
	bool	anglehack;
	char		keyname[ 256 ];
	int			n;

	pEnt = nullptr;

	bool init = false;

	if( !ED_FindClassName( data ) )
	{
		printf( "ED_ParseEdict: couldn't find classname\n" );
		return false;
	}

	CBaseEntity* pEntity = g_EntList.Create( com_token );

	if( !pEntity )
	{
		printf( "ED_ParseEdict: Couldn't create entity '%s'\n", com_token );
		return false;
	}

	// go through all the dictionary pairs
	while( 1 )
	{
		// parse key
		data = COM_Parse( data );
		if( com_token[ 0 ] == '}' )
			break;
		if( !data )
		{
			printf( "ED_ParseEntity: EOF without closing brace\n" );
			g_EntList.Destroy( pEntity );
			return false;
		}

		// anglehack is to allow QuakeEd to write single scalar angles
		// and allow them to be turned into vectors. (FIXME...)
		if( !strcmp( com_token, "angle" ) )
		{
			strcpy( com_token, "angles" );
			anglehack = true;
		}
		else
			anglehack = false;

		// FIXME: change light to _light to get rid of this hack
		if( !strcmp( com_token, "light" ) )
			strcpy( com_token, "light_lev" );	// hack for single light def

		strcpy( keyname, com_token );

		// another hack to fix heynames with trailing spaces
		n = strlen( keyname );
		while( n && keyname[ n - 1 ] == ' ' )
		{
			keyname[ n - 1 ] = 0;
			n--;
		}

		// parse value	
		data = COM_Parse( data );
		if( !data )
		{
			printf( "ED_ParseEntity: EOF without closing brace\n" );
			g_EntList.Destroy( pEntity );
			return false;
		}

		if( com_token[ 0 ] == '}' )
		{
			printf( "ED_ParseEntity: closing brace without data\n" );
			g_EntList.Destroy( pEntity );
			return false;
		}

		init = true;

		// keynames with a leading underscore are used for utility comments,
		// and are immediately discarded by quake
		if( keyname[ 0 ] == '_' )
			continue;

		if( anglehack )
		{
			char	temp[ 32 ];
			strcpy( temp, com_token );
			sprintf( com_token, "0 %s 0", temp );
		}

		if( !pEntity->KeyValue( keyname, com_token ) )
		{
			/*
			printf( "ED_ParseEdict: parse error\n" );
			g_EntList.Destroy( pEntity );
			return false;
			*/
		}
	}

	pszOut = data;

	pEnt = pEntity;

	return true;
}

/*
================
ED_LoadFromFile

The entities are directly placed in the array, rather than allocated with
ED_Alloc, because otherwise an error loading the map would have entity
number references out of order.

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.

Used for both fresh maps and savegame loads.  A fresh map would also need
to call ED_CallSpawnFunctions () to let the objects initialize themselves.
================
*/
bool ED_LoadFromFile( char *data )
{
	int inhibit = 0;

	CBaseEntity* pEnt;

	// parse ents
	while( 1 )
	{
		// parse the opening brace	
		data = COM_Parse( data );
		if( !data )
			break;
		if( com_token[ 0 ] != '{' )
		{
			printf( "ED_LoadFromFile: found %s when expecting {", com_token );
			return false;
		}

		if( !ED_ParseEdict( data, data, pEnt ) )
			return false;

		// remove things from different skill levels or deathmatch
		/*
		if( deathmatch.value )
		{
			if( ( ( int ) ent->v.spawnflags & SPAWNFLAG_NOT_DEATHMATCH ) )
			{
				ED_Free( ent );
				inhibit++;
				continue;
			}
		}
		*/

		pEnt->Spawn();
	}

	printf( "%i entities inhibited\n", inhibit );

	return true;
}

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
			auto header = LoadBSPFile( "external/hldemo2.bsp" );

			memset( BSP::mod_known, 0, sizeof( BSP::mod_known ) );

			m_pModel = &BSP::mod_known[ 0 ];
			BSP::mod_numknown = 1;

			strcpy( m_pModel->name, "external/test.bsp" );

			bSuccess = BSP::LoadBrushModel( m_pModel, header.get() );

			if( bSuccess )
			{
				printf( "Loaded BSP\n" );

				if( ED_LoadFromFile( m_pModel->entities ) )
				{
					//Set up worldspawn.
					g_EntList.GetFirstEntity()->KeyValue( "model", m_pModel->name );
				}
				else
				{
					printf( "Failed to parse entity data\n" );
					bSuccess = false;
					g_EntList.Clear();
				}
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

			g_EntList.Clear();
		}

		BSP::FreeModel( m_pModel );
	}

	//Fetch any remaining errors.
	check_gl_error();

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

	std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() );

	size_t uiCount = 0;

	size_t uiTriangles = 0;

	double flTotal = 0;

	//TODO: only needed for transparent entities. - Solokiller
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );

	for( CBaseEntity* pEntity = g_EntList.GetFirstEntity(); pEntity; pEntity = g_EntList.GetNextEntity( pEntity ) )
	{
		if( auto pModel = pEntity->GetBrushModel() )
		{
			RenderModel( projection, view, model, pEntity, *pModel, uiCount, uiTriangles, flTotal );
		}
	}

	/*
	for( int iIndex = 0; iIndex < BSP::mod_numknown; ++iIndex )
	{
		RenderModel( projection, view, model, BSP::mod_known[ iIndex ], uiCount, uiTriangles, flTotal );
	}
	*/

	std::chrono::milliseconds now2 = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() );

	printf( "Time spent rendering frame (%u polygons, %u triangles, average (msec): %f): %f\n", uiCount, uiTriangles, flTotal / uiCount, ( now2 - now ).count() / 1000.0f );

	//Unbind program
	g_ShaderManager.DeactivateActiveShader();

	check_gl_error();

	//Update screen
	m_pWindow->SwapGLBuffers();

	check_gl_error();
}

void CApp::RenderModel( const glm::mat4x4& projection, const glm::mat4x4& view, const glm::mat4x4& model, const CBaseEntity* pEntity, bmodel_t& brushModel, size_t& uiCount, size_t& uiTriangles, double& flTotal )
{
	msurface_t* pSurface = brushModel.surfaces + brushModel.firstmodelsurface;

	CShaderInstance* pShader = m_pLightmapShader;

	//TODO: need to sort transparent surfaces - Solokiller
	for( int iIndex = 0; iIndex < brushModel.nummodelsurfaces; ++iIndex, ++pSurface )
	{
		//Sky, origin, aaatrigger, etc. Don't draw these.
		//TODO: add option to draw them.
		if( pSurface->texinfo->flags & TEX_SPECIAL )
			continue;

		pShader = pSurface->texinfo->texture->pShader;

		g_ShaderManager.ActivateShader( pShader, projection, view, model, pEntity );

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

				check_gl_error();

				pShader->SetupVertexAttribs();

				check_gl_error();

				pShader->Draw( pPoly2->numverts );

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