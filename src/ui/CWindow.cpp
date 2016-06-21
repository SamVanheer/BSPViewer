#include <cassert>
#include <cstdio>

#include "CWindowArgs.h"

#include "CWindow.h"

CWindow::CWindow( const char* const pszTitle, const int iX, const int iY, const int iWidth, const int iHeight, const bool bShow )
{
	assert( pszTitle );

	Uint32 flags = SDL_WINDOW_OPENGL;

	if( bShow )
	{
		flags |= SDL_WINDOW_SHOWN;
	}

	m_pWindow = SDL_CreateWindow( pszTitle, iX, iY, iWidth, iHeight, flags );
}

CWindow::CWindow( const CWindowArgs& args )
{
	//Always set OpenGL flag.
	m_pWindow = SDL_CreateWindow( args.GetTitle(), args.GetX(), args.GetY(), args.GetWidth(), args.GetHeight(), args.GetFlags() | SDL_WINDOW_OPENGL );
}

CWindow::~CWindow()
{
	if( m_pWindow )
	{
		SDL_DestroyWindow( m_pWindow );
		m_pWindow = nullptr;
	}
}

bool CWindow::IsValid() const
{
	return m_pWindow != nullptr;
}

bool CWindow::IsShown() const
{
	assert( m_pWindow );

	return ( SDL_GetWindowFlags( m_pWindow ) & SDL_WINDOW_SHOWN ) != 0;
}

void CWindow::Show()
{
	assert( m_pWindow );

	SDL_ShowWindow( m_pWindow );
}

void CWindow::Hide()
{
	assert( m_pWindow );

	SDL_HideWindow( m_pWindow );
}

const char* CWindow::GetTitle() const
{
	assert( m_pWindow );

	return SDL_GetWindowTitle( m_pWindow );
}

void CWindow::SetTitle( const char* const pszTitle )
{
	assert( m_pWindow );
	assert( pszTitle );

	SDL_SetWindowTitle( m_pWindow, pszTitle );
}

int CWindow::GetXPos() const
{
	assert( m_pWindow );

	int x;

	SDL_GetWindowPosition( m_pWindow, &x, nullptr );

	return x;
}

void CWindow::SetXPos( int x )
{
	assert( m_pWindow );

	SDL_SetWindowPosition( m_pWindow, x, GetYPos() );
}

int CWindow::GetYPos() const
{
	assert( m_pWindow );

	int y;

	SDL_GetWindowPosition( m_pWindow, nullptr, &y );

	return y;
}

void CWindow::SetYPos( int y )
{
	assert( m_pWindow );

	SDL_SetWindowPosition( m_pWindow, GetXPos(), y );
}

void CWindow::GetPos( int& x, int& y ) const
{
	assert( m_pWindow );

	SDL_GetWindowPosition( m_pWindow, &x, &y );
}

void CWindow::SetPos( int x, int y )
{
	assert( m_pWindow );

	SDL_SetWindowPosition( m_pWindow, x, y );
}

int CWindow::GetWidth() const
{
	assert( m_pWindow );

	int width;

	SDL_GetWindowSize( m_pWindow, &width, nullptr );

	return width;
}

void CWindow::SetWidth( const int width )
{
	assert( m_pWindow );

	SDL_SetWindowSize( m_pWindow, width, GetHeight() );
}

int CWindow::GetHeight() const
{
	assert( m_pWindow );

	int height;

	SDL_GetWindowSize( m_pWindow, nullptr, &height );

	return height;
}

void CWindow::SetHeight( const int height )
{
	assert( m_pWindow );

	SDL_SetWindowSize( m_pWindow, GetWidth(), height );
}

void CWindow::GetSize( int& width, int& height )
{
	assert( m_pWindow );

	SDL_GetWindowSize( m_pWindow, &width, &height );
}

void CWindow::SetSize( int width, int height )
{
	assert( m_pWindow );

	SDL_SetWindowSize( m_pWindow, width, height );
}

Uint32 CWindow::GetFlags() const
{
	assert( m_pWindow );

	return SDL_GetWindowFlags( m_pWindow );
}

void CWindow::MakeFullscreen( const bool bFullscreen )
{
	assert( m_pWindow );

	if( SDL_SetWindowFullscreen( m_pWindow, bFullscreen ? SDL_WINDOW_FULLSCREEN : 0 ) < 0 )
	{
		printf( "CWindow::MakeFullscreen: Failed to change fullscreen mode\nSDL error: %s\n", SDL_GetError() );
	}
}

Uint32 CWindow::GetWindowId() const
{
	assert( m_pWindow );

	return SDL_GetWindowID( m_pWindow );
}

void CWindow::SwapGLBuffers()
{
	assert( m_pWindow );

	SDL_GL_SwapWindow( m_pWindow );
}