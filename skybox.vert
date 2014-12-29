#version 130

in vec3 vpos;
out vec3 fpos;

uniform	mat4 mvmat;
uniform mat4 promat;

uniform vec3 eyepos;
uniform vec3 center;
uniform vec3 up;

uniform samplerCube s1;
uniform samplerCube s2;

void main() {
	vec4 p = vec4(vpos, 1.0);
	//p.x -= eyepos.x;
	//p.y -= eyepos.y;
	//p.z -= eyepos.z;
	gl_Position = promat * (mvmat * p);
	//gl_Position.y += eyepos.y;
	//gl_Position.z += eyepos.z;

	fpos = vpos;
}
