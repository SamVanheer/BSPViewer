#ifndef APP_CAPP_H
#define APP_CAPP_H

#include <chrono>

#include <SDL.h>

#include <gl/glew.h>

#include "bsp/BSPRenderDefs.h"

#include "utility/CCamera.h"

class CWindow;
class CBaseEntity;

/**
*	App class.
*/
class CApp final
{
private:
	static const float ROTATE_SPEED;
	static const float MOVE_SPEED;

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

	/**
	*	Renders a single model.
	*/
	void RenderModel( const glm::mat4x4& projection, const glm::mat4x4& view, const glm::mat4x4& model, const CBaseEntity* pEntity, bmodel_t& brushModel, size_t& uiCount, size_t& uiTriangles, double& flTotal );

	void Event( const SDL_Event& event );

	void KeyEvent( const SDL_KeyboardEvent& event );

	void MouseButtonEvent( const SDL_MouseButtonEvent& event );

	void MouseMotionEvent( const SDL_MouseMotionEvent& event );

	void MouseWheelEvent( const SDL_MouseWheelEvent& event );

private:
	CWindow* m_pWindow = nullptr;

	bmodel_t* m_pModel;

	CCamera m_Camera;

	std::chrono::milliseconds m_StartTime = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() );
	std::chrono::milliseconds m_LastTick;
	std::chrono::milliseconds m_LastFPSCheck;
	float m_flDeltaTime = 0;
	float m_flYawVel = 0;
	float m_flPitchVel = 0;

private:
	CApp( const CApp& ) = delete;
	CApp& operator=( const CApp& ) = delete;
};

#endif //APP_CAPP_H