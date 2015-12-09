#version 430

//in int gl_Layer;

//layout(location=gl_Layer) out;

out float density;
flat in int InstanceID;

void main()
{
	density = -3.0f;
}