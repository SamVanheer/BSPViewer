#include "CBaseShader.h"

/**
*	Draws a polygon.
*/
BEGIN_SHADER( Polygon )

	BEGIN_SHADER_ATTRIBS()
		SHADER_ATTRIB( LVertexPos2D, VEC2 )

		SHADER_OUTPUT( outColor )
	END_SHADER_ATTRIBS()

	SHADER_DRAW
	{
		glDrawArrays( GL_POLYGON, 0, uiNumVerts );

		check_gl_error();
	}

END_SHADER()