#version 430

uniform mat4 model_to_clip_matrix;

in vec3 Vertex;
flat out int InstanceID;

void main()
{
		//InstanceID = gl_InstanceID;
		//vec4 z_offset= vec4(0.0f, 0.0f, 1.0f, 0.0f);
		float conv= 1.0f;//uintBitsToFloat(gl_InstanceID);
		vec4 z_offset= vec4(0.0f, 0.0f, conv, 0.0f);
		gl_Position = model_to_clip_matrix * (vec4(Vertex,1.0f)+z_offset);
}

