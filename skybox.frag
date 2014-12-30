#version 130

in vec3 fpos;

uniform	mat4 mvmat;
uniform mat4 promat;

uniform vec3 eyepos;

uniform samplerCube s1;
uniform samplerCube s2;

void main()
{
	vec3 n = normalize(fpos);

	// wat is this.
	float v = sin(atan(n.x, n.z));
	v *= v;
	
	gl_FragColor = mix(texture(s1, n), texture(s2, n), v);
}
