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
	
	float crunch = 10;
	
	float t = atan(sqrt(1 - n.z * n.z), -n.z);
	t *= crunch;
	
	float ct = cos(t);
	float st = sin(t);

	float p = atan(n.y, n.x);
	float cp = cos(p);
	float sp = sin(p);

	vec3 vp = vec3(st * cp, st * sp, ct);

	//t *= 0;
	//gl_FragColor = t != 0 ? texture(s1, vp) : vec4(0, 0, 0, 1.0);
	//gl_FragColor = t != 0 ? texture(s2, vp) : vec4(0, 0, 0, 1.0);

	gl_FragColor = t != 0.010101 ? texture(s1, n) : vec4(0, 0, 0, 0);
	gl_FragColor = t != 0.010101 ? texture(s2, n) : vec4(0, 0, 0, 0);
}
