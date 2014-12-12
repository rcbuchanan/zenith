#include <err.h>
#include <stdlib.h>

#include <GL/gl.h>

#include "linmath.h"

#include "camera.h"


struct camera {
	mat4x4 projection;
	mat4x4 modelview;
	mat4x4 transform;
};


struct camera *
camera_create()
{
	struct	camera *c;

	if ((c = malloc(sizeof (struct camera))) == NULL)
		errx(1, __FILE__ ": malloc");

	camera_reset_modelview(c);
	camera_set_projection(c, 1.0);

	return c;
}

void
camera_reset_modelview(struct camera *c)
{
	vec3	eye = {0, 0, -2};
	vec3	cent = {0, 0, 0};
	vec3	up = {0, 1, 0};

	mat4x4_look_at(c->modelview, eye, cent, up);
}

void
camera_rotate(struct camera *c, GLfloat x, GLfloat y, GLfloat z)
{
	mat4x4	tmp;

	mat4x4_rotate(tmp, c->modelview, -x, -y, z, 0.001);
	mat4x4_dup(c->modelview, tmp);
	//mat4x4_mul(c->modelview, rot, mv0);
}

void
camera_translate(struct camera *c, GLfloat x, GLfloat y, GLfloat z)
{
	mat4x4_translate_in_place(c->modelview, x, -y, -z);
}

void
camera_set_projection(struct camera *c, GLfloat aspect)
{
	mat4x4_perspective(c->projection, 3.14159 * 90.0 / 360, aspect, 0, 10);
}

GLfloat *
camera_matrix(struct camera *c)
{
	mat4x4_mul(c->transform, c->projection, c->modelview);
	return (GLfloat *) c->transform;
}
