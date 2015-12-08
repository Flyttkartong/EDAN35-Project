#version 330

uniform mat4 model_to_clip_matrix;

in vec3 Vertex;
in vec3 Color;
//in float Sides;


out VS_OUT {
	vec3 Color;
} vs_out;

//out float vSides;

void main()
{
		gl_Position = model_to_clip_matrix * vec4(Vertex,1.0f);
		vs_out.Color = Color;
		//vSides = Sides;
}