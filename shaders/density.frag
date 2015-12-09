#version 430

in vec3 fColor;

out float density;

void main()
{
	density = vec4(fColor, 1.0);
}