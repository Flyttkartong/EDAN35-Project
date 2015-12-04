#version 140

in vec4 Vertex;
in vec4 Color;

out vec4 vColor;
out vec2 vTexcoord;

void main()
{
	gl_Position = Vertex;
	vColor = Color;
	vTexcoord = Vertex.xy;
}


