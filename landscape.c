#include <err.h>
#include <stdlib.h>

#include <gsl/gsl_math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include <GL/glew.h>

#include "linmath.h"

#include "util.h"

#include "filewatcher.h"
#include "matstack.h"
#include "view.h"


#define ABS(x) (((x) > 0) ? (x) : (-(x)))


struct landscape {
	struct	GLvarray *vtx;
	struct	GLbuffer *tri;
};

struct polyhedron {
	GLfloat	*p;
	int	 nps;
	GLuint	*f;
	int	 nfs;
};


static	GLfloat ico_points [12][3] = {
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

static	GLuint ico_faces[20][3] = {
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

static	struct polyhedron ico_polyhedron = {
	.p	= (GLfloat *) ico_points,
	.nps	= 12,
	.f	= (GLuint *) ico_faces,
	.nfs	= 20,
};
	

static	struct GLprogram *line_prog;
static	struct GLprogram *shade_prog;
static	struct watched_program *shwatch;


static	void create_programs();
static	struct polyhedron *polyhedron_createsubdivided(struct polyhedron *);
static	void polyhedron_free(struct polyhedron *);


void
create_programs()
{
	struct	GLshader *vtx;
	struct	GLshader *line;
	struct	GLshader *shade;

	vtx = create_GLshader("./s.vert", GL_VERTEX_SHADER);
	line = create_GLshader("./line.frag", GL_FRAGMENT_SHADER);
	shade = create_GLshader("./shade.frag", GL_FRAGMENT_SHADER);

	line_prog = create_GLprogram();
	addshader_GLprogram(line_prog, vtx);
	addshader_GLprogram(line_prog, line);
	link_GLprogram(line_prog);

	shade_prog = create_GLprogram();
	addshader_GLprogram(shade_prog, vtx);
	addshader_GLprogram(shade_prog, shade);
	link_GLprogram(shade_prog);

	shwatch = create_watched_program(shade_prog);
	if (shwatch == NULL)
		errx(1, __FILE__ ": problem watching file");
}

struct landscape *
landscape_create()
{
	struct	landscape *l;
	struct	polyhedron *p;
	int	 i;
	GLfloat	*fp;
	GLuint	*up;

	if (line_prog == NULL || shade_prog == NULL)
		create_programs();

	if ((l = malloc(sizeof (struct landscape))) == NULL)
		errx(1, __FILE__ ": allocation failed");

	if ((p = polyhedron_createsubdivided(&ico_polyhedron)) == NULL)
		errx(1, __FILE__ ": failed to create polyhedron");

	l->vtx = create_GLvarray(sizeof (GLfloat), 6 * p->nps);
	fp = (GLfloat *) l->vtx->buf->d;
	for (i = 0; i < p->nps; i++) {
		fp[i * 6 + 0] = p->p[i * 3 + 0];
		fp[i * 6 + 1] = p->p[i * 3 + 1];
		fp[i * 6 + 2] = p->p[i * 3 + 2];
		fp[i * 6 + 3] = p->p[i * 3 + 0];
		fp[i * 6 + 4] = p->p[i * 3 + 1];
		fp[i * 6 + 5] = p->p[i * 3 + 2];
	}
	glBindBuffer(GL_ARRAY_BUFFER, l->vtx->buf->id);
	glBufferData(
		GL_ARRAY_BUFFER,
		l->vtx->buf->size,
		l->vtx->buf->d,
		GL_STATIC_DRAW);

	l->tri = create_GLbuffer(sizeof (GLuint), 3 * p->nfs);
	up = (GLuint *) l->tri->d;
	for (i = 0; i < p->nfs * 3; i++)
		up[i] = (GLfloat) p->f[i];
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l->tri->id);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		l->tri->size,
		l->tri->d,
		GL_STATIC_DRAW);

	glBindVertexArray(l->vtx->id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * 2 * sizeof (GLfloat), 0);
	glEnableVertexAttribArray(0);

	polyhedron_free(p);

	return l;
}

void
landscape_draw(struct landscape *l)
{
	update_program(shwatch);

	glBindVertexArray(l->vtx->id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l->tri->id);
	glBindBuffer(GL_ARRAY_BUFFER, l->vtx->buf->id);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * 2 * sizeof (GLfloat), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		3 * 2 * sizeof (GLfloat),
		(void *) (sizeof (GLfloat) * 3));
	glEnableVertexAttribArray(1);

	//glFrontFace(GL_CCW);
	//glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glUseProgram(line_prog->id);
	glUniformMatrix4fv(0, 1, GL_TRUE, (float *) projection_modelview_collapse());

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, l->tri->n * 3 / 4 + 3 * 5, GL_UNSIGNED_INT, 0);

	glUseProgram(shade_prog->id);
	glUniformMatrix4fv(0, 1, GL_TRUE, (float *) projection_modelview_collapse());

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPolygonOffset(1.0, 1.0);

	glDrawElements(GL_TRIANGLES, l->tri->n * 3 / 4 + 3 * 5, GL_UNSIGNED_INT, 0);
	glDisable(GL_POLYGON_OFFSET_FILL);
}

static struct polyhedron *
polyhedron_createsubdivided(struct polyhedron *in)
{
	struct	 polyhedron *out;
	GLuint	 i;

	if ((out = malloc(sizeof (struct polyhedron))) == NULL)
		errx(1, __FILE__ ": malloc");

	if ((out->p = malloc(sizeof (GLfloat) * in->nps * 3)) == NULL)
		errx(1, __FILE__ ": malloc");
	if ((out->f = malloc(sizeof (GLuint) * in->nfs * 3)) == NULL)
		errx(1, __FILE__ ": malloc");

	out->nps = in->nps;
	out->nfs = in->nfs;

	for (i = 0; i < in->nps; i++) {
		out->p[i * 3 + 0] = in->p[i * 3 + 0];
		out->p[i * 3 + 1] = in->p[i * 3 + 1];
		out->p[i * 3 + 2] = in->p[i * 3 + 2];
	}

	for (i = 0; i < in->nfs; i++) {
		out->f[i * 3 + 0] = in->f[i * 3 + 0];
		out->f[i * 3 + 1] = in->f[i * 3 + 1];
		out->f[i * 3 + 2] = in->f[i * 3 + 2];
	}

	return out;
}

static void
polyhedron_free(struct polyhedron *p)
{
	if (p->f)	free(p->f);
	if (p->p)	free(p->p);
	free(p);
}
