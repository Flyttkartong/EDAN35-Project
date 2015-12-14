#version 430

in vec3 fColor;

out vec4 fragColor;

void main()
{
	//fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	//vec3 color = fColor;
	
	vec3 up=vec3(0.0f,1.0f,0.0f);
	
	if(dot(up,fColor)>0.8f)
	{
		fragColor=vec4(0.0f,fColor.y*0.3,0.0f,1.0f);
	} else {
		float multiplier=(fColor.x+fColor.y+fColor.z/3)*0.7;
		fragColor = vec4(multiplier,multiplier,multiplier, 1.0f);
	}
	
	
}