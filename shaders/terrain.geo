#version 430

uniform sampler3D density;
uniform int faces[];
uniform float OriginVertex[];

layout(points) in;
// We need tex_coord here...

layout(triangles, max_vertices = 15) out;

void main()
{
	vec4 origin = vec4(OriginVertex[0], OriginVertex[1], OriginVertex[2], 0.0f);
	vec4 origo = gl_In[0].gl_position;
	float offset = 1.0f;

	// Offset coordinates. ORDER IMPORTANT! See GPU Gems 3, ch 1
	vec4 pos[8];
	pos[0] = origo + vec4(	0.0f, 	0.0f, 	0.0f, 	0.0f);
	pos[1] = origo + vec4(	offset, 0.0f, 	0.0f, 	0.0f);
	pos[2] = origo + vec4(	offset, offset, 0.0f, 	0.0f);
	pos[3] = origo + vec4(	0.0f, 	offset, 0.0f, 	0.0f);
	pos[4] = origo + vec4(	0.0f, 	0.0f, 	offset, 0.0f);
	pos[5] = origo + vec4(	offset, 0.0f, 	offset, 0.0f);
	pos[6] = origo + vec4(	offset, offset, offset, 0.0f);
	pos[7] = origo + vec4(	0.0f, 	offset, offset, 0.0f);
	
	// Represent edges as index pairs
	int (* e)[12];
	e[0] = {0, 1};
	e[0][0] = 0; 	e[0][1] = 1;
	e[1][0] = 1; 	e[1][1] = 2;
	e[2][0] = 2; 	e[2][1] = 3;
	e[3][0] = 3; 	e[3][1] = 0;
	e[4][0] = 4; 	e[4][1] = 5;
	e[5][0] = 5; 	e[5][1] = 6;
	e[6][0] = 6; 	e[6][1] = 7;
	e[7][0] = 7; 	e[7][1] = 4;
	e[8][0] = 0; 	e[8][1] = 4;
	e[9][0] = 1; 	e[9][1] = 5;
	e[10][0] = 3; 	e[10][1] = 7;
	e[11][0] = 2; 	e[11][1] = 6;
	
	// Create sample points by offsetting by origin. ORDER IMPORTANT! See GPU Gems 3, ch 1
	vec3 v[8];
	for(int i = 0; i < 8; i++)
	{
		v[i] = pos[i] + origin;
	}
	
	// Sample density value and create case number by converting from bit to int
	float densities[8];
	int case_nbr = 0;
	for(int i = 0; i < 8; i++)
	{
		densities[i] = texture3D(v[i]);
		if(densities[i] >= 0) 
		{
			case_nbr += pow(2, i);
		}
	}
	
	// Offset case number to get the correct index for faces
	int case_index = case_nbr * 15;
	
	// Fetch edges from faces
	vec4 edges[5];
	int index = 0;
	for(int i = 0; i < 15; i+=3) 
	{
		triangle_points[index] = vec4(face[case_index + i], face[case_index + i + 1], face[case_index + i + 2], 0.0f);
		index++;
	}
	
	// Output vertices
	for(int i = 0; i < 5; i++) 
	{
		// Do 3 times: EmitVertex()
		// EndPrimitive()
	}
	
}