#version 130


uniform	mat4 mvmat;
uniform	mat4 promat;
uniform float t;
uniform samplerCube s1;
uniform samplerCube s2;


void main()
{
	gl_FragColor = vec4(1, 1, 1, 1);
}
