#version 330

uniform mat4 model_to_clip_matrix;

in vec3 Vertex;
in vec3 Color;

out vec3 vColor;

void main()
{
		gl_Position = model_to_clip_matrix * vec4(Vertex,1.0);
		vColor = Color;
}

