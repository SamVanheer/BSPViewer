#include <glm/gtc/matrix_transform.hpp>

#include "CCamera.h"

const float CCamera::MAX_PITCH = glm::pi<float>() / 2.0f;
const float CCamera::MIN_PITCH = -glm::pi<float>() / 2.0f;

CCamera::CCamera( const glm::vec3& vecPosition, float flStartYaw, float flStartPitch )
	: m_vecPosition( vecPosition )
	, m_flYaw( flStartYaw )
	, m_flPitch( flStartPitch )
{
}

float CCamera::ClampPitch( float flPitchInRadians )
{
	if( flPitchInRadians > MAX_PITCH )
		flPitchInRadians = MAX_PITCH;
	else if( flPitchInRadians < MIN_PITCH )
		flPitchInRadians = MIN_PITCH;

	return flPitchInRadians;
}

glm::vec3 CCamera::GetDirection() const
{
	//Apply 180 degrees rotation by default
	float flPitch = glm::pi<float>() + m_flPitch;

	return glm::vec3(
		cos( flPitch ) * sin( m_flYaw ),
		cos( flPitch ) * cos( m_flYaw ),
		sin( flPitch )
	);
}

glm::mat4 CCamera::GetViewMatrix() const
{
	glm::vec3 direction( GetDirection() );

	glm::vec3 right = glm::vec3(
		sin(m_flYaw - 3.14f/2.0f),
		cos(m_flYaw - 3.14f/2.0f),
		0
	);

	glm::vec3 up = glm::cross( right, direction );

	return glm::lookAt(
		m_vecPosition,
		m_vecPosition + direction,
		up
		);
}

void CCamera::RotateYaw( float flAmount )
{
	m_flYaw += glm::radians( flAmount );
}

void CCamera::RotatePitch( float flAmount )
{
	m_flPitch += glm::radians( flAmount );

	m_flPitch = ClampPitch( m_flPitch );
}
