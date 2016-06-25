#version 140

out vec4 outColor;

in vec2 outVecTexCoord;
in vec2 outVecLightmapCoord;

uniform sampler2D tex;
uniform sampler2D lightmap;

void main()
{
	outColor = texture( tex, outVecTexCoord ) * texture( lightmap, outVecLightmapCoord );
/*
	outColor = texture( tex, outVecTexCoord ) / vec4( 0.66666669, 0.66666669, 0.66666669, 1.0 ) + 
	texture( lightmap, outVecLightmapCoord ) / vec4( 0.66666669, 0.66666669, 0.66666669, 1.0 );
	*/
}