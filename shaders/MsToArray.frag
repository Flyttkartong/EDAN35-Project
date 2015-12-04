#version 150

in int gl_SampleID;

uniform sampler2DMS Texture;
layout(binding = 1, rgba32f)	uniform writeonly image2DArray		imageOut;

void main() {
	ivec2 texcoord = ivec2(gl_FragCoord.xy);
	vec4 color = texelFetch(Texture, texcoord, gl_SampleID);
	imageStore(imageOut, ivec3(texcoord, gl_SampleID), color);
}
