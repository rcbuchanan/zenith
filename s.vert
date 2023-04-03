#version 130


in vec3 pos;
in vec3 norm;
out vec3 fnorm;


uniform	mat4 mvmat;
uniform	mat4 promat;
uniform float t;
uniform samplerCube s1;


//vec3 map_norm(in vec3 p)
//{
//	vec2 e = vec2(0.0001, 0);
//	return normalize(vec3(
//		map(p+e.xyy) - map(p-e.xyy),
//		map(p+e.yxy) - map(p-e.yxy),
//		map(p+e.yyx) - map(p-e.yyx)));
//}


void main() {
	//gl_Position = promat * (mvmat * vec4(pos * abs(sin(pos.x * t)), 1));
	gl_Position = promat * (mvmat * vec4(pos, 1));

	vec4 tmp = vec4(norm, 1);
	fnorm = tmp.xyz;
}
