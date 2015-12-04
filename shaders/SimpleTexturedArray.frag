#version 140

in vec2 vTexcoord;

out vec4 fragColor;

uniform sampler2DArray Texture;
uniform int Layer;
uniform ivec4 Swizzle;
uniform int MipBias;

void main() {
	vec2 res = vec2(textureSize(Texture, 0));
	ivec2 texcoord = ivec2(vTexcoord * res);
	vec4 color = texelFetch(Texture, ivec3(ivec2(texcoord), Layer), MipBias);
	for (int i = 0; i < 4; i++)
		fragColor[i] = Swizzle[i] >= 0 && Swizzle[i] <= 3 ? color[Swizzle[i]] : 1.0;
}
