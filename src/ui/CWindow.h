#ifndef UI_CWINDOW_H
#define UI_CWINDOW_H

#include <SDL.h>

class CWindowArgs;

/**
*	A single window.
*/
class CWindow final
{
public:
	/**
	*	Constructor.
	*	@param pszTitle Window title.
	*	@param iX Window X position.
	*	@param iY Window Y position.
	*	@param iWidth Window width.
	*	@param iHeight Window height.
	*	@param bShow Whether to show the window.
	*/
	CWindow( const char* const pszTitle, const int iX, const int iY, const int iWidth, const int iHeight, const bool bShow = true );

	/**
	*	Constructor.
	*	@param args Window arguments.
	*/
	CWindow( const CWindowArgs& args );

	/**
	*	Destructor.
	*/
	~CWindow();

	/**
	*	@return The SDL window.
	*/
	SDL_Window* GetSDLWindow() { return m_pWindow; }

	/**
	*	@return Whether the window is valid.
	*/
	bool IsValid() const;

	/**
	*	@return Whether the window is being shown.
	*/
	bool IsShown() const;

	/**
	*	Show the window.
	*/
	void Show();

	/**
	*	Hide the window.
	*/
	void Hide();

	/**
	*	@return Get window title.
	*/
	const char* GetTitle() const;

	/**
	*	Set window title.
	*	@param pszTitle Title.
	*/
	void SetTitle( const char* const pszTitle );

	/**
	*	@return X position.
	*/
	int GetXPos() const;

	/**
	*	Sets the X position.
	*/
	void SetXPos( int x );

	/**
	*	@return Y position.
	*/
	int GetYPos() const;

	/**
	*	Sets the Y position.
	*/
	void SetYPos( int y );

	/**
	*	Gets the window position.
	*/	
	void GetPos( int& x, int& y ) const;

	/**
	*	Sets the window position.
	*/
	void SetPos( int x, int y );

	/**
	*	@return Width.
	*/
	int GetWidth() const;

	/**
	*	Set width.
	*/
	void SetWidth( const int width );

	/**
	*	@return Height.
	*/
	int GetHeight() const;

	/**
	*	Set height.
	*/
	void SetHeight( const int height );

	/**
	*	@return Window size.
	*/
	void GetSize( int& width, int& height );

	/**
	*	Set window size.
	*/
	void SetSize( int width, int height );

	/**
	*	@return Window flags.
	*/
	Uint32 GetFlags() const;

	/**
	*	Makes the window fullscreen or windowed.
	*/
	void MakeFullscreen( const bool bFullscreen );

	/**
	*	@return Window ID.
	*/
	Uint32 GetWindowId() const;

	/**
	*	Swaps the GL buffers.
	*/
	void SwapGLBuffers();

private:
	SDL_Window* m_pWindow = nullptr;

private:
	CWindow( const CWindow& ) = delete;
	CWindow& operator=( const CWindow& ) = delete;
};

#endif //UI_CWINDOW_H