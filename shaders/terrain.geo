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

	// Offset coordinates. ORDER IMPORTANT!
	vec4 pos[8];
	pos[0] = origo + vec4(	0.0f, 	0.0f, 	0.0f, 	0.0f);
	pos[1] = origo + vec4(	offset, 0.0f, 	0.0f, 	0.0f);
	pos[2] = origo + vec4(	offset, offset, 0.0f, 	0.0f);
	pos[3] = origo + vec4(	0.0f, 	offset, 0.0f, 	0.0f);
	pos[4] = origo + vec4(	0.0f, 	0.0f, 	offset, 0.0f);
	pos[5] = origo + vec4(	offset, 0.0f, 	offset, 0.0f);
	pos[6] = origo + vec4(	offset, offset, offset, 0.0f);
	pos[7] = origo + vec4(	0.0f, 	offset, offset, 0.0f);
	
	// Create data structure
	struct vertexPair
	{
		int a;
		int b;
	}
	
	// Represent edges as vertex index pairs
	vertexPair e[12];
	e[0] = vertexPair(0, 1);
	e[1] = vertexPair(1, 2);
	e[2] = vertexPair(2, 3);
	e[3] = vertexPair(3, 0);
	e[4] = vertexPair(4, 5);
	e[5] = vertexPair(5, 6);
	e[6] = vertexPair(6, 7);
	e[7] = vertexPair(7, 4);
	e[8] = vertexPair(0, 4);
	e[9] = vertexPair(1, 5);
	e[10]= vertexPair(3, 7);
	e[11]= vertexPair(2, 6); 
	
	
	// Create sample points by offsetting by origin.
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
	
	// Fetch edge index from faces and get vertex pair from e
	vertexPair edges[15];
	int edge_index;
	int nbr_edges = 15;
	for(int i = 0; i < 15; i++) 
	{
		edge_index = faces[case_index + i];
		if(edge_index != -1) 
		{
			edges[i] = e[edge_index];
		}
		else
		{
			nbr_edges = i; // We encountered -1, update nbr_edges
			break;
		}
	}
	
	// Interpolate vertex positions and output primitives
	for(int i = 0; i < nbr_edges; i++)
	{
		vec4 a = pos[edges[i].a];
		vec4 b = pos[edges[i].b];
		float a_density = densities[edges[i].a];
		float b_density = densities[edges[i].b];
		
		float b_weight = abs(b_density)/abs(a_density - b_density);
		float a_weight = abs(a_density)/abs(a_density - b_density);
		gl_Position = a * a_weight + b * b_weight;
		
		EmitVertex();
		
		if(i % 3 == 0) // Done with one triangle
		{
			EndPrimitive();
		}
	}
}