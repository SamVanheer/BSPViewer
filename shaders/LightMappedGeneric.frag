#version 140

out vec4 outColor;

in vec2 vecColor;

void main()
{
	outColor = vec4( vecColor, 0.0, 1.0 );
}