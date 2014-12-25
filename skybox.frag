#version 130

in vec3 fpos;

uniform	mat4 mvmat;
uniform samplerCube s1;
uniform samplerCube s2;

void main()
{
	float k = 1.0 / (1 << 9);
	//gl_FragColor = vec4(fpos * k, 1);
	gl_FragColor = mix(texture(s1, fpos), texture(s2, fpos), sin(length(fpos)));
	gl_FragColor.a = 1;

	//gl_FragColor = vec4(1, 1, 1, 1);
	//gl_FragColor = vec4(normalize(abs(fpos)), 1);
}
