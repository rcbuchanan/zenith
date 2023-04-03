#version 130


in vec3 fnorm;


uniform	mat4 mvmat;
uniform	mat4 promat;
uniform float t;
uniform samplerCube s1;
uniform samplerCube s2;


void main()
{
	vec3 nn = normalize(fnorm);
	vec3 src = normalize(vec3(0, 1, 1));

	vec3 col = vec3(0, 1, 0) * clamp(dot(src, nn), 0, 1);
	col = clamp(col + 0.08, 0, 1);

	col = log(floor(exp(col) * 10)) - log(10);
	col = texture(s2, fnorm).xyz;

	gl_FragColor = vec4(col, 1);
}
