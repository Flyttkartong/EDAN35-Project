#version 140

in vec2 vTexcoord;
 
out vec4 fragColor;

uniform sampler2D Texture;
uniform vec4 Color;

void main() {
	fragColor = Color * texture(Texture, vTexcoord);
}
