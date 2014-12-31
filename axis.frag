#version 130


in vec3 fpos;
in vec3 fcol;

uniform	mat4 mvmat;
uniform	mat4 promat;

void main()
{
	gl_FragColor = vec4(fcol, 1);
}
