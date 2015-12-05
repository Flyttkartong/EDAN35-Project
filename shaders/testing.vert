#version 330

uniform mat4 model_to_world_matrix;
uniform mat4 model_to_clip_matrix;

in vec3 Vertex;

out vec3 world_position;

void main() {
	
		world_position = (model_to_world_matrix*vec4(Vertex,1.0)).xyz;
		gl_Position = model_to_clip_matrix * vec4(Vertex,1.0);
}

