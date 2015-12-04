#version 140

in vec2 Vertex;
in vec2 Texcoord;

out vec2 vTexcoord;

uniform ivec2 FlipAxis;

void main()
{
	gl_Position = vec4(Vertex.xy, 0.0, 1.0);
	vTexcoord = Texcoord;
	if (FlipAxis.x < 0)
		vTexcoord.x = 1.0 - vTexcoord.x;
	if (FlipAxis.y < 0)
		vTexcoord.y = 1.0 - vTexcoord.y;

}


