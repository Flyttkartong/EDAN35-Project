#version 430

in vec3 fColor;

out vec4 fragColor;

void main()
{
	//fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	fragColor = vec4(fColor, 1.0f);
}