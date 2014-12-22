#version 130


in vec3 fpos;
in vec3 fcol;

uniform	mat4 mvmat;

void main()
{
	gl_FragColor = vec4(fcol, 1);
	//gl_FragColor = vec4(1, 1, 1, 1);
}
