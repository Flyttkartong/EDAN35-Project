#version 430

uniform mat4 model_to_clip_matrix;

in vec3 vPosition;
in vec3 vTexcoord;

out vec3 fTexcoord;

void main()
{
	gl_Position = model_to_clip_matrix * vec4(vPosition, 1.0f);
	fTexcoord = vTexcoord;
}


