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
	//float ctheta = dot(vec3(0, 0, 1), n); or, if you're not crappy at math...
	float ctheta = n.z;
	float phi = atan(n.y, n.x);
	float r = 4;

	//vec3 vp = normalize(vec3(r * cos(phi * r) + r / 2,
	//     r * sin(phi * r) + r / 2, phi));

	float ct = ctheta;
	float st = sqrt(1 - ctheta * ctheta);

	float theta = atan(st, ct) * 10;
	vec3 vp = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
	//vp = n;
	
	gl_FragColor = theta < 3.14159 ? texture(s1, vp) : vec4(0, 0, 0, 1.0);
	gl_FragColor = theta < 3.14159 * 2 ? texture(s2, vp) : vec4(0, 0, 0, 1.0);
	
}
