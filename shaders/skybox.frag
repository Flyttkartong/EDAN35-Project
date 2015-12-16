#version 430

uniform samplerCube cubemap_texture;

in vec3 fTexcoord;

out vec4 fragColor;

void main() {
	fragColor = texture(cubemap_texture, fTexcoord);
}
