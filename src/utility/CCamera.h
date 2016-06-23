#ifndef UTILITY_CCAMERA_H
#define UTILITY_CCAMERA_H

#include <glm/glm.hpp>

/*
* A 3D camera that represents the viewport
*/
class CCamera
{
public:
	static const float MAX_PITCH;
	static const float MIN_PITCH;

public:
	CCamera( const glm::vec3& vecPosition = glm::vec3(), float flStartYaw = 0.0f, float flStartPitch = 0.0f );

	static float ClampPitch( float flPitchInRadians );

	glm::vec3& GetPosition() { return m_vecPosition; }
	const glm::vec3& GetPosition() const { return m_vecPosition; }

	void SetPosition( const glm::vec3& vecPosition ) { m_vecPosition = vecPosition; }

	glm::vec3 GetDirection() const;

	glm::mat4 GetViewMatrix() const;

	void RotateYaw( float flAmount );

	void RotatePitch( float flAmount );

private:
	glm::vec3 m_vecPosition;
	float m_flYaw;
	float m_flPitch;
};

#endif //UTILITY_CCAMERA_H
