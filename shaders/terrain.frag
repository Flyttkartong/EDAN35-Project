#version 430

in vec3 normal;
in vec2 texCoord;

uniform sampler2D clouds;
uniform float ResX;
uniform float ResY;

out vec4 fragColor;

void main()
{
	//fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	//vec3 color = fColor;
	
	vec3 up=vec3(0.0f,1.0f,0.0f);
	float texClouds=texture(clouds,vec2(texCoord)).x;
	if(dot(up,normal)>0.8f)
	{
		fragColor=vec4(0.0f,normal.y*(0.8*texClouds+0.2),0.0f,1.0f);
	} 
	else 
	{
		float multiplier = (normal.x+normal.y+normal.z/3.0f)*0.7;
		fragColor = vec4(multiplier,multiplier,multiplier, 1.0f);
		
	}
}