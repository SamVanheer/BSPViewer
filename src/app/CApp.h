#ifndef APP_CAPP_H
#define APP_CAPP_H

#include <gl/glew.h>

class CWindow;
class CBaseShader;

/**
*	App class.
*/
class CApp final
{
public:
	/**
	*	Constructor.
	*/
	CApp() = default;

	/**
	*	Destructor.
	*/
	~CApp() = default;

	/**
	*	Runs the app.
	*/
	int Run( int iArgc, char* pszArgV[] );

private:
	/**
	*	Initializes the app.
	*	@return true on success, false otherwise.
	*/
	bool Initialize();

	/**
	*	Shuts down the app and cleans up resources.
	*	Should be called even if Initialize returned false.
	*/
	void Shutdown();

	/**
	*	Runs the app itself.
	*	@return true on success, false otherwise.
	*/
	bool RunApp();

	/**
	*	Renders a frame.
	*/
	void Render();

private:
	CWindow* m_pWindow = nullptr;
	CBaseShader* m_pPolygonShader = nullptr;

	GLuint m_VBO = 0;
	GLuint m_IBO = 0;

private:
	CApp( const CApp& ) = delete;
	CApp& operator=( const CApp& ) = delete;
};

#endif //APP_CAPP_H