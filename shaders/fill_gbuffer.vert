#version 330

uniform mat4 model_to_clip_matrix;
uniform mat4 model_to_world_normal_matrix;

in vec3 Vertex;
in vec3 Normal;
in vec3 Binormal;
in vec3 Tangent;
in vec3 Texcoord;

out vec3 worldspace_normal;
out vec3 worldspace_binormal;
out vec3 worldspace_tangent;
out vec2 pass_texcoords;



void main() {
	worldspace_normal   = normalize((model_to_world_normal_matrix * vec4(Normal, 0.0)).xyz);
	worldspace_tangent  = normalize((model_to_world_normal_matrix * vec4(Tangent, 0.0)).xyz);
	worldspace_binormal = normalize((model_to_world_normal_matrix * vec4(Binormal, 0.0)).xyz);
	
	pass_texcoords = Texcoord.xy;

   	gl_Position = model_to_clip_matrix * vec4(Vertex, 1.0);
}