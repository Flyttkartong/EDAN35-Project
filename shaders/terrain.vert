#version 430

in vec3 Vertex;

void main()
{
	// Output position in world coordinates
	gl_Position = vec4(Vertex,1.0f);
}

