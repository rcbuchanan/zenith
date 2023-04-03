#version 130

in vec3 pos;
in vec3 vcol;

out vec3 fcol;

uniform	mat4 mvmat;
uniform	mat4 promat;

void main() {
	gl_Position = promat * (mvmat * vec4(pos, 1.0));

	fcol = vcol;
}
