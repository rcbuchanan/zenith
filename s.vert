#version 130


in	vec3 pos;

uniform	mat4 mvmat;


void main() {
	//gl_Position = mvmat * vec4(pos, 1.0);
	gl_Position = vec4(pos, 1.0);
	gl_Position *= mvmat;
}
