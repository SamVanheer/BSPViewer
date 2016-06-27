#version 140

out vec4 outColor;

in vec2 outVecTexCoord;
in vec2 outVecLightmapCoord;

uniform sampler2D tex;
uniform sampler2D lightmap;

uniform float renderAmount;

void main()
{
	vec4 texColor = texture( tex, outVecTexCoord );
	
	outColor = texColor * texture( lightmap, outVecLightmapCoord );
	
	outColor.a *= renderAmount;
}