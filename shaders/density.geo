#version 430

layout(triangles, invocations = 32) in;

layout(triangle_strip, max_vertices = 10) out;

void main(){
	vec4 z_offset = vec4(0, 0, (float) gl_InvocationID, 0);
	
	//Generating triangles, counterclockwise

	gl_Position =  gl_in[0].gl_Position + z_offset; //ordinariy point, first
	EmitVertex();

	gl_Position =  gl_in[1].gl_Position + z_offset; //ordinariy point, third
	EmitVertex();
	
	gl_Position =  gl_in[2].gl_Position + z_offset; //ordinariy point, third
	EmitVertex();
	
	EndPrimitive();
}