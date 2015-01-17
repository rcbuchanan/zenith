#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "util.h"

#include "linmath/linmath.h"

#include "matstack.h"
#include "view.h"


void view_create(struct view *v, vec3 eye, vec3 obj, vec3 up)
{
	vec3_dup(v->eye, eye);
	vec3_dup(v->obj, obj);
	vec3_dup(v->up, up);
}

void view_rotate(struct view *v, GLfloat x, GLfloat y, GLfloat z, GLfloat a)
{
	vec3 d;
	vec3 t;
	mat4x4 r, i;

	vec3_sub(d, v->obj, v->eye);

	mat4x4_identity(i);
	mat4x4_rotate(r, i, x, y, z, a);
	mat4x4_mul_vec4(t, r, d);

	vec3_add(v->obj, t, v->eye);

	mat4x4_mul_vec4(t, r, v->up);
	vec3_dup(v->up, t);
}

void view_translate(struct view *v, GLfloat x, GLfloat y, GLfloat z)
{
	vec3 ta, xa, ya, za;

	vec3_sub(ta, v->obj, v->eye);
	vec3_norm(xa, ta);

	vec3_norm(za, v->up);

	vec3_mul_cross(ya, za, xa);

	vec3_scale(xa, xa, x);
	vec3_scale(ya, ya, y);
	vec3_scale(za, za, z);

	vec3_add(ta, xa, ya);
	vec3_add(ta, ta, za);

	vec3_add(v->eye, v->eye, ta);
	vec3_add(v->obj, v->obj, ta);
}

void view_update(struct view *v)
{
	modelview_lookat(v->eye, v->obj, v->up);
}
