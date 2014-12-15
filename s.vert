#version 130


in vec3 pos;
in vec3 norm;
out vec3 fnorm;


uniform	mat4 mvmat;


void main() {
	//gl_Position = mvmat * vec4(pos, 1.0);
	gl_Position = vec4(pos, 1.0);
	gl_Position *= mvmat;

	vec4 tmp = vec4(norm, 1.0);
	tmp *= mvmat;
	fnorm = tmp.xyz;
}
