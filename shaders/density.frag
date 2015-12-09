#version 430

in int gl_layer;

layout(location=gl_layer) out;

out float density;

void main()
{
	density = vec4(fColor, 1.0);
}