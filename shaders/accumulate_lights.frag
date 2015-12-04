#version 330

uniform sampler2D depthBuffer;
uniform sampler2D normalAndSpecularBuffer;
uniform sampler2DShadow shadowMap;

uniform vec2 invRes;

uniform mat4 ViewProjectionInverse;
uniform mat4 ViewProjection;
uniform vec3 ViewPosition;
uniform mat4 shadowViewProjection;

uniform vec3 LightColor;
uniform vec3 LightPosition;
uniform vec3 LightDirection;
uniform float LightIntensity;
uniform float LightAngleFalloff;

uniform vec2 ShadowMapTexelSize;

in vec4 gl_FragCoord;

layout (location = 0) out vec4 light_diffuse_contribution;
layout (location = 1) out vec4 light_specular_contribution;


void main()
{
	
	vec2 XY =gl_FragCoord.xy;
	vec2 texcoords=XY;
	texcoords.x*=invRes.x; //[0..resX] -> [0..1]
	texcoords.y*=invRes.y; //[0..resY] -> [0..1]
	vec3 normal = 2.0*texture(normalAndSpecularBuffer,texcoords).xyz-1.0;
	float spec = texture(normalAndSpecularBuffer,texcoords).w;
	
	float depth = 2.0*texture(depthBuffer,texcoords).x-1.0;
	
	vec2 world_xy = 2.0*texcoords-1.0; // from [0..1] -> [-1..1]
	vec4 incorrect_world_position = (ViewProjectionInverse*vec4(world_xy,depth,1.0));
	vec3 world_position=incorrect_world_position.xyz/incorrect_world_position.w;
	
	vec3 nLightDirection = -LightDirection;
	nLightDirection=normalize(nLightDirection);
	vec3 lightvector = LightPosition-world_position;
	float distance_falloff = 1.0/dot(lightvector,lightvector);
	lightvector = normalize(lightvector);

	vec3 view_vector= normalize(ViewPosition-world_position);
	vec3 R=normalize(reflect(-lightvector,normal));
	float phong = max(dot(lightvector,normal),0.0)+spec*pow(max(dot(view_vector,R),0.0),100);
    


	
	float cos_theta=dot(lightvector,nLightDirection);
	float theta = acos(cos_theta);
	float falloff_angle = 3.1415926538/6.0;
	float angular_falloff = max((theta-falloff_angle)/(-falloff_angle),0.0); //[45..0] -> [0..1]
	
	/*vec4 colortest= vec4(0.0,0.0,0.0,0.0);
	
		colortest.y=1.0;
		colortest.y*=cos_theta;
	*/
	
	
	vec4 incorrect_light_surface = shadowViewProjection*incorrect_world_position;
	vec3 light_surface = incorrect_light_surface.xyz/incorrect_light_surface.w;
	light_surface=light_surface*0.5+0.5;
	int endpts=3;
	int samplepts= (endpts*2+1)*(endpts*2+1);
	float shadow_multiplier=0.0;
	vec3 sample;
	float x = 0.0;
	float y = 0.0;
	for(int i = -endpts; i <= endpts; i++)
	{
		x = i * ShadowMapTexelSize.x;
		for(int j = -endpts; j<=endpts; j++)
		{
			y = j * ShadowMapTexelSize.y;
			if (light_surface.x+x>0.0 && light_surface.x+x<1.0 && light_surface.y+y>0.0 && light_surface.y+y<1.0) {
				sample = vec3(light_surface.x+x, light_surface.y+y, light_surface.z);
				shadow_multiplier += texture(shadowMap, sample);
			} else {
				samplepts--;
			}
		}
	}
	//float shadow_multiplier = texture(shadowMap, light_surface*0.5+0.5);
	shadow_multiplier=shadow_multiplier/samplepts;
	
	
	light_diffuse_contribution = LightIntensity*shadow_multiplier*distance_falloff*angular_falloff*vec4(LightColor,0.0)*max(dot(lightvector,normal),0.0);
	light_specular_contribution = LightIntensity*shadow_multiplier*distance_falloff*angular_falloff*vec4(LightColor,0.0)*spec*pow(max(dot(view_vector,R),0.0),100);
}