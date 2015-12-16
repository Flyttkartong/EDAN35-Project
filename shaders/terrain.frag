#version 430

in vec3 normal;
in vec2 texCoord;

uniform sampler2D clouds;

out vec4 fragColor;

void main()
{
	// Use surface angle to determine if grass or rock
	vec3 up = vec3(0.0f, 1.0f, 0.0f);
	float texClouds = texture(clouds, vec2(texCoord)).x; // Sample grass color
	if(dot(up, normal) > 0.8f) // Grass
	{
		fragColor = vec4(
			0.0f, 
			normal.y * (0.8f * texClouds + 0.2f), 
			0.0f, 
			1.0f
		);
	} 
	else // Rock
	{
		float multiplier = (normal.x + normal.y + normal.z / 3.0f) * 0.7f;
		fragColor = vec4(multiplier, multiplier, multiplier, 1.0f);
	}
}