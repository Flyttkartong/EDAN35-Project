
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


void main()
{
	//TODO: add color, reflection, animated bump mapping, refraction and fresnel

	//Define colors
	 vec4 Color_deep = vec4(0.0,0.0,0.1,1.0);
	 vec4 Color_shallow = vec4(0.0,0.5,0.5,1.0);

	vec3 V = normalize(View);

	 vec3 N = normalize(Normal);

	// Bump maps
	 vec4 bumpNormal=normalize(texture(BumpMapTexture,bumpCoord0)*2-1+texture(BumpMapTexture,bumpCoord1)*2-1+texture(BumpMapTexture,bumpCoord2)*2-1);

	 mat3 BTN = mat3(Binormal, Tangent, Normal);

	 vec3 newNormal = BTN*bumpNormal.xyz;

	// Reflection
	vec3 R = normalize(-reflect(V,newNormal.xyz));

	float facing = 1.0 - max(dot(V,newNormal),0);

	vec4 Color_water = mix(Color_deep, Color_shallow,facing);

	vec4 Color_reflection = vec4(0.0,0.0,0.2,1);//0.8*texture(SkyboxTexture,R);

	// Fresnel
	float R0 = 0.02037; //Air to water
	float fresnel = R0 + (1.0-R0)*pow((1.0-dot(V,newNormal)),5.0);
    
	// Refraction
	vec3 refraction = vec3(refract(-V,newNormal,1.0/1.33));
	vec4 Color_refraction = vec4(0,0,0.8,1);//texture(SkyboxTexture,refraction);

	/*Final Color*/

	 fragColor = Color_water + Color_reflection * fresnel + Color_refraction * (1-fresnel); 
	//fragColor=vec4(1.0f,0.0f,0.0f,1.0f);
}