#include "CBaseShader.h"

/**
*	Draws a lightmapped polygon with alphe testing. Used by '{' textures to make solid blue fully transparent.
*/
BEGIN_SHADER( LightMappedAlphaTest )

BEGIN_SHADER_ATTRIBS()
SHADER_ATTRIB( vecPosition, VEC3 )
SHADER_ATTRIB( vecTexCoord, VEC2 )
SHADER_ATTRIB( vecLightmapCoord, VEC2 )

SHADER_UNIFORM( tex, SAMPLER_TEXTURE )
SHADER_UNIFORM( lightmap, SAMPLER_TEXTURE )

SHADER_OUTPUT( outColor )

END_SHADER_ATTRIBS()

	SHADER_DRAW
	{
		glDrawArrays( GL_POLYGON, 0, uiNumVerts );

		check_gl_error();
	}

END_SHADER()