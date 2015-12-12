#version 430

in vec3 Vertex;

void main()
{
	gl_Position = vec4(Vertex,1.0f);
}

