#version 130


in vec3 fnorm;


uniform	mat4 mvmat;
uniform	mat4 promat;
uniform samplerCube s1;

void main()
{
	vec3 nn = normalize(fnorm);
	vec3 src = normalize(vec3(0, 1, 1));

	vec3 col = vec3(0, 1, 0) * clamp(dot(src, nn), 0, 1);
	col = clamp(col + 0.08, 0, 1);

	col = log(floor(exp(col) * 10)) - log(10);
	col = texture(s1, fnorm).xyz;

	gl_FragColor = vec4(col, 1);
}
