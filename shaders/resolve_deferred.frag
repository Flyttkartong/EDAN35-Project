#version 330

uniform sampler2D diffuse_buffer;
uniform sampler2D specular_buffer;
uniform sampler2D light_d_buffer;
uniform sampler2D light_s_buffer;

out vec4 frag_color;

void main()
{
	vec4 diffuse = texelFetch(diffuse_buffer, ivec2(gl_FragCoord.xy), 0);
	if (diffuse.a < 0.2)
		discard;

	      vec3 light_d  = texelFetch(light_d_buffer,  ivec2(gl_FragCoord.xy), 0).rgb;
	      vec3 light_s  = texelFetch(light_s_buffer,  ivec2(gl_FragCoord.xy), 0).rgb;
	      vec3 specular = texelFetch(specular_buffer, ivec2(gl_FragCoord.xy), 0).rgb;
	const vec3 ambient = vec3(0.2);

	frag_color =  vec4((ambient + light_d) * diffuse.rgb + light_s * specular, 1.0);
}
