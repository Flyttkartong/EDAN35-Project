#version 430

in vec4 pos;

out float density; // Do something with layout and gl_Layer

void main()
{
	float y = -(pos.y - 16.0f) / 32.0f; // From [0..32] to [-16..16] to [-0.5..0.5]
	density = y + 0.5f; // [0..1]
}