#include <err.h>
#include <stdlib.h>

#include <gsl/gsl_math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include <GL/glew.h>

#include "util.h"


#define ABS(x) (((x) > 0) ? (x) : (-(x)))


#define DELTA	0.02
#define RES	200
#define PEAK	0.025


struct landscape {
	struct	GLvarray *vtx;
	struct	GLbuffer *tri;
};


GLfloat points [12][3] = {
	{ 0.00000000,  0.00000000, -0.95105650},
	{ 0.00000000,  0.85065080, -0.42532537},
	{ 0.80901698,  0.26286556, -0.42532537},
	{ 0.50000000, -0.68819095, -0.42532537},
	{-0.50000000, -0.68819095, -0.42532537},
	{-0.80901698,  0.26286556, -0.42532537},
	{ 0.50000000,  0.68819095,  0.42532537},
	{ 0.80901698, -0.26286556,  0.42532537},
	{ 0.00000000, -0.85065080,  0.42532537},
	{-0.80901698, -0.26286556,  0.42532537},
	{-0.50000000,  0.68819095,  0.42532537},
	{ 0.00000000,  0.00000000,  0.95105650}
};

GLuint faces[20][3] = {
	{0,  2,  1},
	{0,  3,  2},
	{0,  4,  3},
	{0,  5,  4},
	{0,  1,  5},
	{1,  6, 10},
	{1,  2,  6},
	{2,  7,  6},
	{2,  3,  7},
	{3,  8,  7},
	{3,  4,  8},
	{4,  9,  8},
	{4,  5,  9},
	{5, 10,  9},
	{5,  1, 10},
	{10, 6, 11},
	{6,  7, 11},
	{7,  8, 11},
	{8,  9, 11},
	{9, 10, 11}
};


static	void generate_points(struct landscape *);
static	void generate_triangles(struct landscape *);


struct landscape *
landscape_create()
{
	struct	landscape *l;

	if ((l = malloc(sizeof (struct landscape))) == NULL)
		errx(1, __FILE__ ": allocation failed");

	l->vtx = create_GLvarray(sizeof (GLfloat), 3 * sizeof (points) / sizeof (points[0]));
	l->tri = create_GLbuffer(sizeof (GLuint), 3 * sizeof (faces) / sizeof (faces[0]));

	generate_points(l);
	generate_triangles(l);

	glBindBuffer(GL_ARRAY_BUFFER, l->vtx->buf->id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l->tri->id);

	glBufferData(
		GL_ARRAY_BUFFER,
		l->vtx->buf->size,
		l->vtx->buf->d,
		GL_STATIC_DRAW);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		l->tri->size,
		l->tri->d,
		GL_STATIC_DRAW);

	glBindVertexArray(l->vtx->id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	return l;
}

void
landscape_draw(struct landscape *l)
{
	glBindVertexArray(l->vtx->id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l->tri->id);

	//glFrontFace(GL_CCW);
	//glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glDrawElements(GL_TRIANGLES, l->tri->n * 3 / 4 + 3 * 5, GL_UNSIGNED_INT, 0);
	//glDrawArrays(GL_POINTS, 0, l->vtx->buf->n);
}


static void
generate_points(struct landscape *l)
{
	GLfloat	*f;
	GLuint	 i;

	f = (GLfloat *) l->vtx->buf->d;

	for (i = 0; i < l->vtx->buf->n / 3; i++) {
		f[i * 3 + 0] = points[i][0];
		f[i * 3 + 1] = points[i][1];
		f[i * 3 + 2] = points[i][2] + 1;
		//f[i * 3 + 2] = 1;
		printf("%f %f %f\n", f[i * 3 + 0], f[i * 3 + 1], f[i * 3 + 2]);
	}
}

void
generate_triangles(struct landscape *l)
{
	GLuint	*u;
	GLuint	 i;

	u = (GLuint *) l->tri->d;

	for (i = 0; i < l->tri->n / 3; i++) {
		u[i * 3 + 0] = faces[i][0];
		u[i * 3 + 1] = faces[i][1];
		u[i * 3 + 2] = faces[i][2];
	}

}
