#ifndef UI_CWINDOWMANAGER_H
#define UI_CWINDOWMANAGER_H

#include <vector>

class CWindow;
class CWindowArgs;

/**
*	Window manager.
*/
class CWindowManager final
{
private:
	typedef std::vector<CWindow*> Windows_t;

public:
	CWindowManager();
	~CWindowManager();

	/**
	*	@return Whether the manager is initialized.
	*/
	bool IsInitialized() const { return m_bInitialized; }

	/**
	*	@return The result returned by the first call to Initialize.
	*/
	bool InitResult() const { return m_bInitResult; }

	/**
	*	Initializes the manager.
	*	@return Whether the manager successfully initialized.
	*/
	bool Initialize();

	/**
	*	Shuts down the manager.
	*/
	void Shutdown();

	/**
	*	Creates a new window.
	*	@param pszTitle Window title.
	*	@param iX Window X position.
	*	@param iY Window Y position.
	*	@param iWidth Window width.
	*	@param iHeight Window height.
	*	@param bShow Whether to show the window.
	*	@return The window, or null if an error occurred.
	*/
	CWindow* CreateWindow( const char* const pszTitle, const int iX, const int iY, const int iWidth, const int iHeight, const bool bShow = true );

	/**
	*	Creates a new window. The window is centered onscreen.
	*	@param pszTitle Window title.
	*	@param iWidth Window width.
	*	@param iHeight Window height.
	*	@param bShow Whether to show the window.
	*	@return The window, or null if an error occurred.
	*/
	CWindow* CreateWindow( const char* const pszTitle, const int iWidth, const int iHeight, const bool bShow = true );

	/**
	*	Creates a new window.
	*	@return The window, or null if an error occurred.
	*/
	CWindow* CreateWindow( const CWindowArgs& args );

private:
	CWindow* WindowCreated( CWindow* pWindow );

public:
	/**
	*	Destroys a window.
	*/
	void DestroyWindow( CWindow* pWindow );

	/**
	*	Hides all windows.
	*/
	void HideAllWindows();

	/**
	*	Destroys all windows.
	*/
	void DestroyAllWindows();

	/**
	*	Makes the GL context current for the given window.
	*/
	void MakeGLCurrent( CWindow* pWindow );

private:
	bool m_bInitialized = false;
	bool m_bInitResult = false;

	/*
	*	Window used to create the GL context.
	*/
	CWindow* m_pContextWindow = nullptr;

	SDL_GLContext m_GLContext = nullptr;

	Windows_t m_Windows;

private:
	CWindowManager( const CWindowManager& ) = delete;
	CWindowManager& operator=( const CWindowManager& ) = delete;
};

extern CWindowManager g_WindowManager;

#endif //UI_CWINDOWMANAGER_H