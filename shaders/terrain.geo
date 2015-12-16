#version 430

const float DENSITY_SIZE_FLOAT = 33.0f;

uniform mat4 model_to_clip_matrix;

uniform sampler3D density_texture;
uniform isampler1D edge_texture;

layout(points) in;

layout(triangle_strip, max_vertices = 15) out;

out vec3 fColor;
out vec3 normal;
out vec2 texCoord;

void main()
{
	vec4 vertex = gl_in[0].gl_Position;
	float offset = 1.0f;

	// Offset vertex coordinate to get cube corners ORDER IMPORTANT! See GPU Gems 3, ch 1
	vec4 pos[8];
	pos[0] = vertex; // (0, 0, 0) corner
	pos[3] = vertex + vec4(	0.0f, 	offset, 0.0f, 	0.0f);
	pos[2] = vertex + vec4(	offset, offset, 0.0f, 	0.0f);
	pos[1] = vertex + vec4(	offset, 0.0f, 	0.0f, 	0.0f);
	pos[4] = vertex + vec4(	0.0f, 	0.0f, 	offset, 0.0f);
	pos[7] = vertex + vec4(	0.0f, 	offset, offset, 0.0f);
	pos[6] = vertex + vec4(	offset, offset, offset, 0.0f);
	pos[5] = vertex + vec4(	offset, 0.0f, 	offset, 0.0f);
	
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
	e[10][0] = 2; 	e[10][1] = 6;
	e[11][0] = 3; 	e[11][1] = 7;
	
	// Create texture sample points
	vec3 v[8];
	for(int i = 0; i < 8; i++)
	{
		v[i] = pos[i].xyz / DENSITY_SIZE_FLOAT;
	}
	
	// Sample density value in corners and create case number by converting from bit to int. ORDER IMPORTANT! See GPU Gems 3, ch 1
	float densities[8];
	int case_nbr = 0;
	for(int i = 0; i < 8; i++)
	{
		densities[i] = texture(density_texture, v[i]).r;
		if(densities[i] >= 0) 
		{
			case_nbr += int(pow(2, i));
		}
	}
	
	// Offset case number to get the correct index for edge_texture
	int case_index = case_nbr * 15;
	
	// Fetch edge index from edge_texture and use it to get vertex pair from e[]
	int edges[15][2];
	int edge_index;
	int nbr_edges = 15;
	for(int i = 0; i < 15; i++) 
	{
		edge_index = texelFetch(edge_texture, case_index + i, 0).r;
		if(edge_index != -1) 
		{
			edges[i][0] = e[edge_index][0];
			edges[i][1] = e[edge_index][1];
		}
		else
		{
			nbr_edges = i; // We encountered -1, update nbr_edges
			break;
		}
	}
	
	// Interpolate vertex positions from density values on the edges
	vec4 out_points[15];
	for(int i = 0; i < nbr_edges; i++)
	{
		out_points[i] = pos[edges[i][0]] * abs(densities[edges[i][0]])/abs(densities[edges[i][0]] - densities[edges[i][1]]) + pos[edges[i][1]] * abs(densities[edges[i][1]])/abs(densities[edges[i][0]] - densities[edges[i][1]]);
	}
	
	// Create normal for each triangle and output triangle strip + texture coordinates
	vec3 U, V;
	int point_index;
	int nbr_triangles = int(nbr_edges / 3);
	for(int i = 0; i < nbr_triangles; i++) 
	{
		point_index = 3 * i;
		
		U = out_points[point_index + 1].xyz - out_points[point_index].xyz;
		V = out_points[point_index + 2].xyz - out_points[point_index].xyz;
		normal = normalize(cross(U, V));
		
		// Vertex 1
		texCoord = vec2(
			out_points[point_index].x / DENSITY_SIZE_FLOAT,
			out_points[point_index].z / DENSITY_SIZE_FLOAT
		);
		gl_Position = model_to_clip_matrix * out_points[point_index];
		EmitVertex();
		
		// Vertex 2
		texCoord = vec2(
			out_points[point_index + 1].x / DENSITY_SIZE_FLOAT,
			out_points[point_index + 1].z / DENSITY_SIZE_FLOAT
		);
		gl_Position = model_to_clip_matrix * out_points[3*i+1];
		EmitVertex();
		
		// Vertex 3
		texCoord = vec2(
			out_points[point_index + 2].x / DENSITY_SIZE_FLOAT,
			out_points[point_index + 2].z/DENSITY_SIZE_FLOAT
		);
		gl_Position = model_to_clip_matrix * out_points[3*i+2];
		EmitVertex();
		
		// Done with triangle
		EndPrimitive();
	}
}















