#version 140

in vec3 Vertex;
in vec2 Texcoord;
in vec2 Scale;
in float Rotation;

out vec2 vTexcoord;

uniform mat4 WorldToClip;
uniform mat4 ClipToWorld;
uniform vec3 FaceDir;
uniform vec3 UpDir;

void main()
{
	float sn = sin(Rotation);
	float cs = cos(Rotation);
	mat2 r = mat2(cs, sn, -sn, cs);
	vec4 x, y;
	if (length(FaceDir) == 0.0) {
		vec4 n = ClipToWorld * vec4(0.0f, 0.0f, -1.0f, 1.0f);
		x = ClipToWorld * vec4(1.0, 0.0, -1.0f, 1.0f);
		y = ClipToWorld * vec4(0.0, 1.0, -1.0f, 1.0f);
		n.xyz /= n.w;
		x.xyz /= x.w;
		y.xyz /= y.w;
		x = normalize(x - n);
		y = normalize(y - n);
	} else { // TODO: Rotate
		y = vec4(normalize(cross(FaceDir, UpDir)), 1.0);
		x = vec4(normalize(UpDir), 1.0);
	}
	vec2 t = -1.0 + 2.0 * Texcoord;
	vec3 p = Vertex + 0.5 * (t.x * Scale.x * x.xyz + t.y * Scale.y * y.xyz);
	gl_Position = WorldToClip * vec4(p, 1.0);
	vTexcoord = Texcoord;
}


