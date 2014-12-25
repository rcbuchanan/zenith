#version 130

in vec3 vpos;
out vec3 fpos;

uniform	mat4 mvmat;
uniform samplerCube s1;
uniform samplerCube s2;

void main() {
	gl_Position = vec4(vpos * 20, 1.0);
	gl_Position *= mvmat;

	fpos = vpos;
}
