#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 10) out;

//in vec3 vColor[];
//in float vSides[];

in VS_OUT {
	vec3 Color;
} gs_in[];

out vec3 fColor;


const float PI = 3.1415926;

void main(){

	vec4 zerotwoBytwo = vec4(0.0f,0.0f,0.0f,0.0f);//vec4(0.0f,0.0f,0.2f,0.0f);
	vec4 zerooneBytwo = vec4(0.0f,0.0f,0.0f,0.0f);//vec4(0.0f,0.0f,-0.2f,0.0f);
	vec4 onetwoBytwo = vec4(0.0f,0.0f,0.0f,0.0f);//vec4(0.0f,0.0f,0.3f,0.0f);
	

	//Generating triangles, counterclockwise

	fColor = gs_in[0].Color;
	gl_Position =  gl_in[0].gl_Position; //ordinariy point, first
	EmitVertex();
/*
	fColor = vec3(1.0f,1.0f,1.0f);
	gl_Position =  (gl_in[0].gl_Position+gl_in[1].gl_Position)/2+zerooneBytwo; //offset downwards
	EmitVertex();

	gl_Position =  (gl_in[0].gl_Position+gl_in[2].gl_Position)/2+zerotwoBytwo ; //offset right/upwards
	EmitVertex();

	gl_Position =  (gl_in[1].gl_Position+gl_in[2].gl_Position)/2+onetwoBytwo; //offset right/upwards
	EmitVertex();

	gl_Position =  (gl_in[0].gl_Position+gl_in[1].gl_Position)/2+zerooneBytwo; //offset downwards
	EmitVertex();*/

	fColor = gs_in[1].Color;
	gl_Position =  gl_in[1].gl_Position; //ordinariy point, third
	EmitVertex();

	/*fColor = vec3(1.0f,1.0f,1.0f);
	gl_Position =  (gl_in[1].gl_Position+gl_in[2].gl_Position)/2+onetwoBytwo; //offset downwards
	EmitVertex();
*/
	fColor = gs_in[2].Color;
	gl_Position =  gl_in[2].gl_Position; //ordinariy point, third
	EmitVertex();
/*
	fColor = vec3(1.0f,1.0f,1.0f);
	gl_Position =  (gl_in[0].gl_Position+gl_in[2].gl_Position)/2+zerotwoBytwo ; //offset right/upwards
	EmitVertex();
*/
	EndPrimitive();


}



/*void build_house(vec4 position)
{   
	fColor = gs_in[0].Color; 
    gl_Position = position + vec4(-0.2f, -0.2f, 0.0f, 0.0f);    // 1:bottom-left
    EmitVertex();   
    gl_Position = position + vec4( 0.2f, -0.2f, 0.0f, 0.0f);    // 2:bottom-right
    EmitVertex();
    gl_Position = position + vec4(-0.2f,  0.2f, 0.0f, 0.0f);    // 3:top-left
    EmitVertex();
    gl_Position = position + vec4( 0.2f,  0.2f, 0.0f, 0.0f);    // 4:top-right
    EmitVertex();
    gl_Position = position + vec4( 0.0f,  0.4f, 0.0f, 0.0f);    // 5:top
    EmitVertex();
    EndPrimitive();
}

void main() {    
    build_house(gl_in[0].gl_Position);	
} */



/*void main() {
    fColor = vColor[0];

	gl_Position = 


    for (int i = 0; i <= vasi; i++) {
        // Angle between each side in radians
        float ang = PI * 2.0 / vSides[0] * i;

        // Offset from center of point (0.3 to accomodate for aspect ratio)
        vec4 offset = vec4(cos(ang) * 0.3, -sin(ang) * 0.4, 0.0, 0.0);
        gl_Position = gl_in[0].gl_Position + offset;

        EmitVertex();
    }

    EndPrimitive();
}*/