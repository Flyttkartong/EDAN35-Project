#version 330

uniform mat4 model_to_world_matrix;
uniform mat4 model_to_clip_matrix;


out vec4 fragColor;

void main(){
	fragColor = vec4(1.0,0.0,0.0,1.0);
}