
#version 330

out vec4 fragColor;

/* Cube- and Bumpmaps */
//uniform samplerCube SkyboxTexture;
uniform sampler2D   BumpMapTexture;

/* Tangent basis */
in vec3	Binormal,
		Tangent,
		Normal;
				
/* View vector */
in vec3	View;

/* Multiple bump coordinates for animated bump mapping */
in vec2	bumpCoord0,
		bumpCoord1,
		bumpCoord2;

in vec3 position;
		
void main()
{
	//TODO: add color, reflection, animated bump mapping, refraction and fresnel

	//Define colors
	 vec4 Color_deep = vec4(0.0f, 0.0f, 0.1f, 1.0f);
	 vec4 Color_shallow = vec4(0.0f, 0.5f, 0.5f, 1.0f);

	vec3 V = normalize(View);

	 vec3 N = normalize(Normal);

	// Bump maps
	 vec4 bumpNormal = normalize(texture(BumpMapTexture, bumpCoord0) * 2 - 1 + texture(BumpMapTexture, bumpCoord1) * 2 - 1 + texture(BumpMapTexture, bumpCoord2) * 2 - 1);

	 mat3 BTN = mat3(Binormal, Tangent, Normal);

	 vec3 newNormal = BTN * bumpNormal.xyz;

	// Reflection
	vec3 R = normalize(-reflect(V, newNormal.xyz));

	float facing = 1.0 - max(dot(V, newNormal), 0.0f);

	vec4 Color_water = mix(Color_deep, Color_shallow, facing);

	vec4 Color_reflection = 0.5f * vec4(0.53f, 0.8f, 0.98f, 1.0f);

	// Fresnel
	float R0 = 0.02037; //Air to water
	float fresnel = R0 + (1.0f - R0) * pow((1.0f - dot(V, newNormal)), 5.0f);
    
	// Refraction
	vec3 refraction = vec3(refract(-V, newNormal, 1.0f / 1.33f));
	vec4 Color_refraction = vec4(0.0f, 0.0f, 0.2f, 1.0f);

	// Create foam around island coordinates
	vec4 foam = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	float foam_radius = (abs(15.0f - position.z) * abs(15.0f - position.z) + abs(15.0f - position.x) * abs(15.0f - position.x) + 0.2f) / (30 * 30);
	foam = 1.0f / (foam_radius * foam_radius) * vec4(0.05f, 0.05f, 0.05f, 1.0f);
	
	// Water color to output
	vec4 out_water_color = Color_water + Color_reflection * fresnel + Color_refraction * (1 - fresnel) + foam; 
	
	// Fade water into clear color around island
	float world_radius = (abs(15.0f - position.z) * abs(15.0f - position.z) + abs(15.0f - position.x) * abs(15.0f - position.x) + 0.2f) / (80 * 80);
	float distance_multiplier = min(1.0f / (world_radius * world_radius), 1.0f);
	fragColor = distance_multiplier * out_water_color + (1 - distance_multiplier) * vec4(0.7f, 0.8f, 0.9f, 1.0f);
}