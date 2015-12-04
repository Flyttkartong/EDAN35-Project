#version 140

in vec2 vTexcoord;

out vec4 fragColor;

uniform sampler2D Texture;
uniform ivec4 Swizzle;
uniform int MipBias;

void main() {
	vec2 res = vec2(textureSize(Texture, 0));
	ivec2 texcoord = ivec2(vTexcoord * res);
	vec4 color = texelFetch(Texture, texcoord, MipBias);
	for (int i = 0; i < 4; i++)
		fragColor[i] = Swizzle[i] >= 0 && Swizzle[i] <= 3 ? color[Swizzle[i]] : 1.0;
}
