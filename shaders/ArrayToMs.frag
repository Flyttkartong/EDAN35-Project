#version 150

in int gl_SampleID;

uniform sampler2DArray Texture;

void main() {
	ivec2 texcoord = ivec2(gl_FragCoord.xy);
	vec4 color = texelFetch(Texture, ivec3(texcoord, gl_SampleID), 0);
	imageStore(imageOut, texcoord, gl_SampleID, color);
}
