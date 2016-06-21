#ifndef UI_CWINDOWARGS_H
#define UI_CWINDOWARGS_H

#include <cassert>

#include <SDL.h>

/**
*	Builder for window arguments.
*/
class CWindowArgs final
{
public:
	/**
	*	Constructor.
	*/
	CWindowArgs() = default;

	/**
	*	@return Title.
	*/
	const char* GetTitle() const { return m_pszTitle; }

	/**
	*	Set title.
	*/
	CWindowArgs& Title( const char* const pszTitle )
	{
		assert( pszTitle );

		m_pszTitle = pszTitle;

		return *this;
	}

	/**
	*	@return X position.
	*/
	int GetX() const { return m_iX; }

	/**
	*	Center X position.
	*/
	CWindowArgs& CenterX()
	{
		m_iX = SDL_WINDOWPOS_CENTERED;

		return *this;
	}

	/**
	*	Set X position.
	*/
	CWindowArgs& X( const int iX )
	{
		m_iX = iX;

		return *this;
	}

	/**
	*	@return Y position.
	*/
	int GetY() const { return m_iY; }

	/**
	*	Center Y position.
	*/
	CWindowArgs& CenterY()
	{
		m_iY = SDL_WINDOWPOS_CENTERED;

		return *this;
	}

	/**
	*	Set Y position.
	*/
	CWindowArgs& Y( const int iY )
	{
		m_iY = iY;

		return *this;
	}

	/**
	*	Center position.
	*/
	CWindowArgs& Center()
	{
		m_iX = SDL_WINDOWPOS_CENTERED;
		m_iY = SDL_WINDOWPOS_CENTERED;

		return *this;
	}
	
	/**
	*	@return Width.
	*/
	int GetWidth() const { return m_iWidth; }

	/**
	*	Set width.
	*/
	CWindowArgs& Width( const int iWidth )
	{
		m_iWidth = iWidth;

		return *this;
	}

	/**
	*	@return Height.
	*/
	int GetHeight() const { return m_iHeight; }

	/**
	*	Set height.
	*/
	CWindowArgs& Height( const int iHeight )
	{
		m_iHeight = iHeight;

		return *this;
	}

	/**
	*	Set size.
	*/
	CWindowArgs& Size( const int iWidth, const int iHeight )
	{
		m_iWidth = iWidth;
		m_iHeight = iHeight;

		return *this;
	}

	/**
	*	@return Flags.
	*/
	Uint32 GetFlags() const { return m_Flags; }

	/**
	*	Set flags.
	*/
	CWindowArgs& Flags( const Uint32 flags )
	{
		m_Flags = flags;

		return *this;
	}

	/**
	*	Add flags.
	*/
	CWindowArgs& AddFlags( const Uint32 flags )
	{
		m_Flags |= flags;

		return *this;
	}

	/**
	*	Show or hide window.
	*/
	CWindowArgs& Show( const bool bShow = true )
	{
		if( bShow )
		{
			m_Flags |= SDL_WINDOW_SHOWN;
		}
		else
		{
			m_Flags &= SDL_WINDOW_SHOWN;
		}

		return *this;
	}

private:
	const char* m_pszTitle = "";

	int m_iX = SDL_WINDOWPOS_UNDEFINED;
	int m_iY = SDL_WINDOWPOS_UNDEFINED;

	int m_iWidth = 640;
	int m_iHeight = 480;

	Uint32 m_Flags = 0;
};

#endif //UI_CWINDOWARGS_H