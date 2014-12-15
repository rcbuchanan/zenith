#version 130


uniform	mat4 mvmat;

/*
 * return distance to surface from point. 0 on surface
 */
float map(in vec3 p)
{
	float a = .2;
	vec3 c1 = vec3(1.6, 0, 0);
	float d1 = length(p + c1) - a;

	return d1;
}

/*
 * return the normal to map at point p
 */
vec3 map_norm(in vec3 p)
{
	vec2 e = vec2(0.0001, 0);

	return normalize(vec3(
		map(p+e.xyy) - map(p-e.xyy),
		map(p+e.yxy) - map(p-e.yxy),
		map(p+e.yyx) - map(p-e.yyx)));
}

void main()
{
	gl_FragColor = vec4(1, 0, 0, 1);
}

//void main()
//{
//	// p is frag pos from +/- 1 area
//	vec2 res = vec2(512, 512);
//	vec2 uv = gl_FragCoord.xy / res;
//	vec2 p = 2 * uv - 1;
//	p *= res.x / res.y;
//
//	// r0 is the max distance out on Z axis
//	vec3 r0 = vec3(0, 0, 2);
//
//	// rd points at the screen point in the negative z direction
//	vec3 rd = normalize(vec3(p, -1));
//	vec3 col = vec3(0);
//
//	float tmax = 20;
//	float h = 1.0;
//	float t = 0.0;
//	for (int i = 0; i < 100; i++) {
//		if (h < 0.0001 || t > tmax)	break;
//
//		// r0 + t * rd is point in the frustrum
//		h = map(r0 + t * rd);
//		t += h;
//	}
//
//	vec3 ls = normalize(vec3(1,1,1));
//	if (t < tmax) {
//		vec3 pos = r0 + t * rd;
//		vec3 norm = map_norm(pos);
//		col = vec3(1, 0, 1) * clamp(dot(norm, ls), 0, 1);
//		col *= norm.z;
//		col += 0.05;
//	}
//
//	gl_FragColor = vec4(col, 1);
//}
