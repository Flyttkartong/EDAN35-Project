#version 430

in vec4 pos;

out float density; // Do something with layout and gl_Layer

void main()
{
	density = -pos.y;
}