#include <chrono>

#include "CShaderInstance.h"

#include "CBaseShader.h"

const auto startTime = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() );

/**
*	Draws a lightmapped polygon with water warping.
*/
BEGIN_SHADER( LightMappedWater )

	BEGIN_SHADER_ATTRIBS()
		SHADER_ATTRIB( vecPosition, VEC3 )
		SHADER_ATTRIB( vecTexCoord, VEC2 )
		SHADER_ATTRIB( vecLightmapCoord, VEC2 )

		SHADER_UNIFORM( realtime, FLOAT )

		SHADER_UNIFORM( tex, SAMPLER_TEXTURE )
		SHADER_UNIFORM( lightmap, SAMPLER_TEXTURE )

		SHADER_OUTPUT( outColor )

	END_SHADER_ATTRIBS()

	SHADER_ACTIVATE
	{
		//Use a relative time here.
		//TODO: track realtime globally.
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() );

		auto curTime = time - startTime;

		float flTime = curTime.count() / 1000.0f;

		glUniform1f( pInstance->GetUniforms()[ realtime ], flTime );
	}

	SHADER_DRAW
	{
		glDrawArrays( GL_POLYGON, 0, uiNumVerts );

		check_gl_error();
	}

END_SHADER()