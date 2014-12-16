#version 130


in vec3 pos;
in vec3 norm;
out vec3 fnorm;


uniform	mat4 mvmat;
uniform float t;

float map(in vec3 p)
{
	return 1 + sin(length(pos) * t * 0.1) * 0.1;
}

vec3 map_norm(in vec3 p)
{
	vec2 e = vec2(0.0001, 0);

	return normalize(vec3(
		map(p+e.xyy) - map(p-e.xyy),
		map(p+e.yxy) - map(p-e.yxy),
		map(p+e.yyx) - map(p-e.yyx)));
}


void main() {
	//gl_Position = mvmat * vec4(pos, 1.0);
	gl_Position = vec4(pos * map(pos), 1.0);
	gl_Position *= mvmat;

	vec4 tmp = vec4(norm, 1.0);
	//vec4 tmp = vec4(map_norm(pos), 1);
	tmp *= mvmat;
	fnorm = tmp.xyz;
}
