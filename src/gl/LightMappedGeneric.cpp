#include "entity/CBaseEntity.h"

#include "CShaderInstance.h"

#include "CBaseShader.h"

/**
*	Draws a lightmapped polygon.
*/
BEGIN_SHADER( LightMappedGeneric )

	BEGIN_SHADER_ATTRIBS()
		SHADER_ATTRIB( vecPosition, VEC3 )
		SHADER_ATTRIB( vecTexCoord, VEC2 )
		SHADER_ATTRIB( vecLightmapCoord, VEC2 )

		SHADER_UNIFORM( tex, SAMPLER_TEXTURE )
		SHADER_UNIFORM( lightmap, SAMPLER_TEXTURE )
		SHADER_UNIFORM( renderAmount, FLOAT )

		SHADER_OUTPUT( outColor )

	END_SHADER_ATTRIBS()

	SHADER_ACTIVATE
	{
		float flRenderAmount = 255;

		if( pEntity->GetRenderMode() == RenderMode::TEXTURE || pEntity->GetRenderMode() == RenderMode::ADDITIVE )
		{
			glEnable( GL_BLEND );
			glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

			flRenderAmount = pEntity->GetRenderAmount();

			if( pEntity->GetRenderMode() == RenderMode::TEXTURE )
			{
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				glColor4f( 1.0, 1.0, 1.0, flRenderAmount );
			}
			else
			{
				glBlendFunc( GL_SRC_ALPHA, GL_ONE );
				glColor4f( flRenderAmount, flRenderAmount, flRenderAmount, 1.0 );
			}
		}
		else
		{
			glDisable( GL_BLEND );
			glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
		}

		glUniform1f( pInstance->GetUniforms()[ renderAmount ], flRenderAmount / 255.0f );
	}

	SHADER_DRAW
	{
		glDrawArrays( GL_POLYGON, 0, uiNumVerts );

		check_gl_error();
	}

END_SHADER()