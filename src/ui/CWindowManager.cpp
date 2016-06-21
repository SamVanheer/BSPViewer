#include <algorithm>
#include <cassert>
#include <cstdio>

#include <GL\glew.h>

#include "CWindow.h"
#include "CWindowArgs.h"

#include "CWindowManager.h"

CWindowManager g_WindowManager;

CWindowManager::CWindowManager()
{
}

CWindowManager::~CWindowManager()
{
}

bool CWindowManager::Initialize()
{
	if( m_bInitialized )
	{
		return m_bInitResult;
	}

	m_bInitialized = true;

	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize!\nSDL error: %s\n", SDL_GetError() );
		return false;
	}

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	m_pContextWindow = new CWindow( CWindowArgs().Title( "GL Context" ).Size( 0, 0 ).Flags( SDL_WINDOW_HIDDEN ) );

	if( !m_pContextWindow )
		return false;

	m_GLContext = SDL_GL_CreateContext( m_pContextWindow->GetSDLWindow() );

	if( !m_GLContext )
		return false;

	glewExperimental = GL_TRUE;

	const GLenum glewResult = glewInit();

	if( glewResult != GLEW_OK )
	{
		printf( "Failed to initialize GLEW\nGLEW error: %s\n", glewGetErrorString( glewResult ) );

		return false;
	}

	if( SDL_GL_SetSwapInterval( 1 ) < 0 )
	{
		printf( "Warning: Unable to set up VSync\nSDL error: %s\n", SDL_GetError() );
	}

	m_bInitResult = true;

	return true;
}

void CWindowManager::Shutdown()
{
	if( !m_bInitialized )
		return;

	m_bInitResult = false;
	m_bInitialized = false;

	DestroyAllWindows();

	if( m_GLContext )
	{
		SDL_GL_DeleteContext( m_GLContext );
		m_GLContext = nullptr;
	}

	if( m_pContextWindow )
	{
		delete m_pContextWindow;
		m_pContextWindow = nullptr;
	}

	DestroyAllWindows();

	SDL_Quit();
}

CWindow* CWindowManager::CreateWindow( const char* const pszTitle, const int iX, const int iY, const int iWidth, const int iHeight, const bool bShow )
{
	return WindowCreated( new CWindow( pszTitle, iX, iY, iWidth, iHeight, bShow ) );
}

CWindow* CWindowManager::CreateWindow( const char* const pszTitle, const int iWidth, const int iHeight, const bool bShow )
{
	return CreateWindow( pszTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, iWidth, iHeight, bShow );
}

CWindow* CWindowManager::CreateWindow( const CWindowArgs& args )
{
	return WindowCreated( new CWindow( args ) );
}

CWindow* CWindowManager::WindowCreated( CWindow* pWindow )
{
	assert( pWindow );

	if( !pWindow->IsValid() )
	{
		printf( "CWindowManager::CreateWindow: Failed to create window!\n" );

		delete pWindow;
		return nullptr;
	}

	m_Windows.push_back( pWindow );

	return pWindow;
}

void CWindowManager::DestroyWindow( CWindow* pWindow )
{
	assert( pWindow );

	if( !pWindow )
		return;

	auto it = std::find( m_Windows.begin(), m_Windows.end(), pWindow );

	if( it == m_Windows.end() )
	{
		printf( "CWindowManager::DestroyWindow: Window is not managed by this manager!\n" );
		return;
	}

	m_Windows.erase( it );

	delete pWindow;
}

void CWindowManager::HideAllWindows()
{
	for( auto pWindow : m_Windows )
	{
		pWindow->Show();
	}
}

void CWindowManager::DestroyAllWindows()
{
	for( auto pWindow : m_Windows )
	{
		delete pWindow;
	}

	m_Windows.clear();
}

void CWindowManager::MakeGLCurrent( CWindow* pWindow )
{
	assert( pWindow );

	if( !pWindow )
		return;

	SDL_GL_MakeCurrent( pWindow->GetSDLWindow(), m_GLContext );
}