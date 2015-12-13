#version 430

const float densitySizeFloat = 33.0f;

uniform mat4 model_to_clip_matrix;

uniform sampler3D Density_texture;
uniform isampler1D Faces_texture;
uniform float OriginVertexX;
uniform float OriginVertexY;
uniform float OriginVertexZ;

layout(points) in;

layout(line_strip, max_vertices = 15) out;

out vec3 fColor;

void main()
{
	vec4 origin = vec4(OriginVertexX, OriginVertexY, OriginVertexZ, 0.0f);
	vec4 vertex = gl_in[0].gl_Position;
	float offset = 1.0f;

	// Offset vertex coordinate to get cube corners ORDER IMPORTANT!
	vec4 pos[8];
	pos[0] = vertex;
	pos[1] = vertex + vec4(	offset, 0.0f, 	0.0f, 	0.0f);
	pos[2] = vertex + vec4(	offset, offset, 0.0f, 	0.0f);
	pos[3] = vertex + vec4(	0.0f, 	offset, 0.0f, 	0.0f);
	pos[4] = vertex + vec4(	0.0f, 	0.0f, 	offset, 0.0f);
	pos[5] = vertex + vec4(	offset, 0.0f, 	offset, 0.0f);
	pos[6] = vertex + vec4(	offset, offset, offset, 0.0f);
	pos[7] = vertex + vec4(	0.0f, 	offset, offset, 0.0f);
	
	// Create data structure for vertex index pairs	
	int e[12][2];
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
	
	
	// Create texture sample points
	vec3 v[8];
	for(int i = 0; i < 8; i++)
	{
		v[i] = pos[i].xyz / densitySizeFloat;//(pos[i] - origin).xyz;
	}
	
	// Sample density value in corners and create case number by converting from bit to int. ORDER IMPORTANT!
	float densities[8];
	int case_nbr = 0;
	for(int i = 0; i < 8; i++)
	{
		densities[i] = texture(Density_texture, v[i]).x;
		if(densities[i] >= 0) 
		{
			case_nbr += int(pow(2, i));
		}
	}
	
	// Offset case number to get the correct index for Faces_texture
	int case_index = 0;// case_nbr * 15;
	
	// Fetch edge index from Faces_texture and get vertex pair from e
	int edges[15][2];
	int edge_index;
	int nbr_edges = 15;
	for(int i = 0; i < 15; i++) 
	{
		edge_index = texelFetch(Faces_texture, case_index + i, 0).x;
		if(edge_index != -1) 
		{
			gl_Position = model_to_clip_matrix * vertex + vec4((0.01f * i),0,0,0);
			fColor = vec3(edge_index/11.f, 0, 0);
			EmitVertex();
			//edges[i][0] = e[edge_index][0];
			//edges[i][1] = e[edge_index][1];
		}
		else
		{
			nbr_edges = i; // We encountered -1, update nbr_edges
			break;
		}
	}
	EndPrimitive();
	/*
	// Interpolate vertex positions and output primitives
	for(int i = 0; i < nbr_edges; i++)
	{
		// vec4 a = pos[edges[i].a];
		// vec4 b = pos[edges[i].b];
		// float a_density = densities[edges[i].a];
		// float b_density = densities[edges[i].b];
		// float b_weight = abs(b_density)/abs(a_density - b_density);
		// float a_weight = abs(a_density)/abs(a_density - b_density);
		// gl_Position = a * a_wight + b * b_weight;
		
		// gl_Position = pos[edges[i].a] * abs(densities[edges[i].a])/abs(densities[edges[i].a] - densities[edges[i].b]) + pos[edges[i].b] * abs(densities[edges[i].b])/abs(densities[edges[i].a] - densities[edges[i].b]);
		
		gl_Position = model_to_clip_matrix * (pos[edges[i][0]] * abs(densities[edges[i][0]])/abs(densities[edges[i][0]] - densities[edges[i][1]]) + pos[edges[i][1]] * abs(densities[edges[i][1]])/abs(densities[edges[i][0]] - densities[edges[i][1]]));
		EmitVertex();
		
		if(i % 3 == 0) // Done with one triangle
		{
			EndPrimitive();
		}
	}*/
}