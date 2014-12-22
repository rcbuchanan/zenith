#version 130

in vec3 pos;
in vec3 vcol;

out vec3 fcol;

uniform	mat4 mvmat;

void main() {
	gl_Position = vec4(pos, 1.0);
	gl_Position *= mvmat;

	fcol = vcol;
}
