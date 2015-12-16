
#version 330

/* Per-vertex input from application */
in vec3 vPosition;
in vec2 vTexCoord;

/* Uniform input from application */
/* Same value for all vertices */
uniform float time;
uniform mat4 ModelViewProjectionMatrix;
uniform vec3 vCameraPos;

/* Tangent basis */
out vec3	Binormal,
			Tangent,
			Normal;
				
/* View vector */
out vec3	View;

/* Multiple bump coordinates for animated bump mapping */
out vec2	bumpCoord0,
			bumpCoord1,
			bumpCoord2;


void main()
{
	vec4 P = vec4(vPosition, 1.0);
	
	// /* TODO: Add waves to P */

	// /*Parameters*/
	vec2 amp = vec2(1.0,0.5);
	vec2 freq = vec2(0.2,0.4);
	vec2 phase = vec2(0.5,1.3);
	vec2 sharp = vec2(2.0,2.0);
	vec2 dir1 = vec2(-1.0,0.0);
	vec2 dir2 = vec2(-0.7,0.7);

	// /*Generate sine functions*/

	float Gfunc1 = pow(amp.x * ( sin( (dir1.x*vPosition.x + dir1.y*vPosition.z) * freq.x + time) * 0.5 + 0.5),sharp.x);
	float Gfunc2 = pow(amp.x * ( sin( (dir2.x*vPosition.x + dir2.y*vPosition.z) * freq.y + time) * 0.5 + 0.5),sharp.y);

	float Hfunc = Gfunc1 + Gfunc2+3.f;

	float HdiffX = amp.x/2 * sharp.x * pow(sin( (dir1.x*vPosition.x + dir1.y*vPosition.z) * freq.x + time),sharp.x-1) * cos((dir1.x*vPosition.x + dir1.y*vPosition.z) * freq.x + time) * dir1.x * freq.x
					+ amp.y/2 * sharp.y * pow(sin( (dir2.x*vPosition.z + dir2.y*vPosition.z) * freq.y + time),sharp.y-1) * cos((dir2.x*vPosition.x + dir2.y*vPosition.z) * freq.y + time) * dir2.x * freq.y;

	float HdiffZ = amp.x/2 * sharp.x * pow(sin( (dir1.x*vPosition.x + dir1.y*vPosition.z) * freq.x + time),sharp.x-1) * cos((dir1.x*vPosition.x + dir1.y*vPosition.z) * freq.x + time) * dir1.y * freq.x
					+ amp.y/2 * sharp.y * pow(sin( (dir2.x*vPosition.z + dir2.y*vPosition.z) * freq.y + time),sharp.y-1) * cos((dir2.x*vPosition.x + dir2.y*vPosition.z) * freq.y + time) * dir2.y * freq.y;

	P.y = Hfunc;
    
	/* TODO: Compute B, T, N */

	/*Calc View vector*/
	View = vCameraPos - vPosition;

	Binormal = vec3(1.0,0.0,HdiffX);

	Tangent = vec3(0.0,1.0,HdiffZ);

	Normal = vec3(-HdiffX, 1.0, -HdiffZ);
    
	/* TODO: Compute bumpmap coordinates */

	vec2 texScale = vec2(8,4);
	float bumpTime = mod(time,100.0);
	vec2 bumpSpeed = vec2(-0.05,0);

	bumpCoord0.xy = vTexCoord.xy*texScale + bumpTime*bumpSpeed;
	bumpCoord1.xy = vTexCoord.xy*texScale*2 + bumpTime*bumpSpeed*4;
	bumpCoord2.xy = vTexCoord.xy*texScale*4 + bumpTime*bumpSpeed*8;
	
	gl_Position = ModelViewProjectionMatrix * P;
}