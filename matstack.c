#include <err.h>
#include <stdlib.h>

#include <GL/gl.h>

#include "linmath.h"

#include "matstack.h"


#define MAX_SIZE	10


struct matstack {
	mat4x4	*d;
	mat4x4	*cache;
	int	 i;
};


static	mat4x4 mvdata[MAX_SIZE];
static	mat4x4 prodata[MAX_SIZE];
static	mat4x4 mvcache[MAX_SIZE];
static	mat4x4 procache[MAX_SIZE];
static	mat4x4 collapsedmat;

static	struct matstack mvstack = {mvdata, mvcache, -1};
static	struct matstack prostack = {prodata, procache, -1};


static	void matstack_update_cache(struct matstack *);
static	void matstack_loadident(struct matstack *);
static	void matstack_pushident(struct matstack *);
static	void matstack_pop(struct matstack *);
static	void matstack_lookat(struct matstack *, vec3, vec3, vec3);
static	void matstack_translate(struct matstack *, GLfloat, GLfloat, GLfloat);
static	void matstack_rotate(struct matstack *, GLfloat, GLfloat, GLfloat, GLfloat);
static	void matstack_set_perspective(struct matstack *, GLfloat, GLfloat, GLfloat, GLfloat);
static	GLfloat *matstack_collapse(struct matstack *);


static void
matstack_update_cache(struct matstack *ms)
{
	if (ms->i < 0)
		return;
	else if (ms->i == 0)
		mat4x4_dup(ms->cache[ms->i], ms->d[ms->i]);
	else
		mat4x4_mul(ms->cache[ms->i], ms->d[ms->i - 1], ms->d[ms->i]);
}

static void
matstack_loadident(struct matstack *ms)
{
	mat4x4_identity(ms->d[ms->i]);
	matstack_update_cache(ms);
}

static void
matstack_pushident(struct matstack *ms)
{
	if (++ms->i > MAX_SIZE)
		errx(1, __FILE__ ": overflowed stack");
	mat4x4_identity(ms->d[ms->i]);
	matstack_update_cache(ms);
}

static void
matstack_pop(struct matstack *ms)
{
	if (--ms->i < 0)
		errx(1, __FILE__ ": underflowed stack");
}

static void
matstack_lookat(struct matstack *ms, vec3 eye, vec3 obj, vec3 up)
{
	mat4x4_look_at(ms->d[ms->i], eye, obj, up);
	matstack_update_cache(ms);
}

static void
matstack_translate(struct matstack *ms, GLfloat x, GLfloat y, GLfloat z)
{
	mat4x4_translate_in_place(ms->d[ms->i], x, -y, -z);
	matstack_update_cache(ms);
}

static void
matstack_rotate(struct matstack *ms, GLfloat x, GLfloat y, GLfloat z, GLfloat angle)
{
	mat4x4 t;

	mat4x4_rotate(t, ms->d[ms->i], x, y, z, angle);
	mat4x4_dup(ms->d[ms->i], t);
	matstack_update_cache(ms);
}

static void
matstack_set_perspective(struct matstack *ms, GLfloat fov, GLfloat aspect, GLfloat n, GLfloat f)
{
	mat4x4_perspective(ms->d[ms->i], 3.14159 * fov / 360, aspect, n, f);
	matstack_update_cache(ms);
}

static GLfloat *
matstack_collapse(struct matstack *ms)
{
	if (ms->i >= 0)
		return (GLfloat *) ms->cache[ms->i];
	errx(1, __FILE__ ": matstack is empty!!");
	return NULL;
}


GLfloat *
modelview_collapse()
{
	return matstack_collapse(&mvstack);
}

void
modelview_pop()
{
	matstack_pop(&mvstack);
}

void
modelview_pushident()
{
	matstack_pushident(&mvstack);
}

void
modelview_loadident()
{
	matstack_loadident(&mvstack);
}

void
modelview_lookat(vec3 eye, vec3 obj, vec3 up)
{
	matstack_lookat(&mvstack, eye, obj, up);
}

void
modelview_translate(GLfloat x, GLfloat y, GLfloat z)
{
	matstack_translate(&mvstack, x, y, z);
}

void
modelview_rotate(GLfloat x, GLfloat y, GLfloat z, GLfloat angle)
{
	matstack_rotate(&mvstack, x, y, z, angle);
}

GLfloat *
projection_collapse()
{
	return matstack_collapse(&prostack);
}

void
projection_pop()
{
	matstack_pop(&prostack);
}

void
projection_pushident()
{
	matstack_pushident(&prostack);
}

void
projection_loadident()
{
	matstack_loadident(&prostack);
}

void
projection_set_perspective(GLfloat fov, GLfloat aspect, GLfloat n, GLfloat f)
{
	matstack_set_perspective(&prostack, fov, aspect, n, f);
}

GLfloat *
projection_modelview_collapse()
{
	mat4x4_mul(
		collapsedmat,
		(GLfloat (*)[4]) matstack_collapse(&prostack),
		(GLfloat (*)[4]) matstack_collapse(&mvstack));
	return (GLfloat *) collapsedmat;
}
