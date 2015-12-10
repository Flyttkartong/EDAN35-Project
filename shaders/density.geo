#version 430

layout(triangles) in;

in VS_OUT {
	int instance_id;
} gs_in[];

layout(triangle_strip, max_vertices = 3) out;

out vec4 pos;

void main(){
	//Generating triangles, counterclockwise
	gl_Position =  gl_in[0].gl_Position; //ordinariy point, first
	pos = gl_Position;
	EmitVertex();

	gl_Position =  gl_in[1].gl_Position; //ordinariy point, third
	pos = gl_Position;
	EmitVertex();
	
	gl_Position =  gl_in[2].gl_Position; //ordinariy point, third
	pos = gl_Position;
	EmitVertex();
	
	//Select layer
	gl_Layer = gs_in[0].instance_id;
	
	EndPrimitive();
}