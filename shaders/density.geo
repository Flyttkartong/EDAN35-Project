#version 430

layout(triangles) in;

layout(triangle_strip, max_vertices = 10) out;

flat in int InstanceID[3];

//flat out int gl_Layer;

void main(){
	
	float conv= float() 
	vec4 z_offset = vec4(0.0f, 0.0f, 1.0f/*(float) gl_InvocationID*/, 0.0f);
	gl_Layer= InstanceID[0]; //gl_InvocationID;
	
	//Generating triangles, counterclockwise

	gl_Position =  gl_in[0].gl_Position + z_offset; //ordinariy point, first
	EmitVertex();

	gl_Position =  gl_in[1].gl_Position + z_offset; //ordinariy point, third
	EmitVertex();
	
	gl_Position =  gl_in[2].gl_Position + z_offset; //ordinariy point, third
	EmitVertex();
	
	//Select layer
	
	
	EndPrimitive();
}