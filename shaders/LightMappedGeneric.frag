#version 140

out vec4 outColor;

in vec2 outVecTexCoord;

uniform sampler2D tex;

void main()
{
	outColor = texture( tex, outVecTexCoord ) * vec4( 1.0, 1.0, 1.0, 1.0 ); //vec4( vecColor, 0.0, 1.0 );
}