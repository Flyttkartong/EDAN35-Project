#version 330

uniform mat4 model_to_clip_matrix;

in vec3 Vertex;

void main()
{
	gl_Position = model_to_clip_matrix * vec4(Vertex, 1.0);
}



