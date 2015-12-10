#version 430

uniform mat4 model_to_clip_matrix;

in vec3 Vertex;

//flat out int instance_id;

out VS_OUT {
	int instance_id;
} vs_out;

void main()
{
	gl_Position = model_to_clip_matrix * vec4(Vertex,1.0f);
	vs_out.instance_id = gl_InstanceID;
}