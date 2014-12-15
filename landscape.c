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


#define DELTA	0.02
#define RES	200
#define PEAK	0.025


struct landscape {
	struct	GLvarray *vtx;
	struct	GLbuffer *tri;
};

struct polyhedron {
	GLfloat	*p;
	int	 ps;
	GLuint	*f;
	int	 fs;
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

static	struct polyhedron = {
	.p	= (GLfloat *) ico_points,
	.ps	= 12
	

static	struct GLprogram *line_prog;
static	struct GLprogram *shade_prog;
static	struct watched_program *watched;


static	void subdivide_polyhedron(const GLfloat *, int, const GLuint *);
static	void create_programs();


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

	watched = create_watched_program(shade_prog);
	if (watched == NULL)
		errx(1, __FILE__ ": problem watching file");
}

struct landscape *
landscape_create()
{
	struct	landscape *l;

	if (line_prog == NULL || shade_prog == NULL)
		create_programs();

	if ((l = malloc(sizeof (struct landscape))) == NULL)
		errx(1, __FILE__ ": allocation failed");

	subdivide_polyhedron(l, (GLfloat *) ico_points, 12, 2);

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
	update_program(watched);

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


static void
subdivide_polyhedron(struct landscape *l, GLfloat *ps, int nps, GLuint *fs, int nfs)
{
	GLfloat	*f;
	GLuint	 i;
	GLuint	*u;

	l->vtx = create_GLvarray(sizeof (GLfloat), 3 * 2 * nps);
	l->tri = create_GLbuffer(sizeof (GLuint), 3 * nfs);

	f = (GLfloat *) l->vtx->buf->d;
	u = (GLuint *) l->tri->d;

	for (i = 0; i < l->tri->n / 3; i++) {
		u[i * 3 + 0] = ico_faces[i][0];
		u[i * 3 + 1] = ico_faces[i][1];
		u[i * 3 + 2] = ico_faces[i][2];
		printf("%d %d %d\n", u[i * 3 + 0], u[i * 3 + 1], u[i * 3 + 2]);
	}

	for (i = 0; i < l->vtx->buf->n / (3 * 2); i++) {
		f[i * 3 * 2 + 0] = ico_points[i][0];
		f[i * 3 * 2 + 1] = ico_points[i][1];
		f[i * 3 * 2 + 2] = ico_points[i][2];
		f[i * 3 * 2 + 3] = ico_points[i][0];
		f[i * 3 * 2 + 4] = ico_points[i][1];
		f[i * 3 * 2 + 5] = ico_points[i][2];

		printf("%f %f %f %f %f %f\n", 
			f[i * 6 + 0],
			f[i * 6 + 1],
			f[i * 6 + 2],
			f[i * 6 + 3],
			f[i * 6 + 4],
			f[i * 6 + 5]
	}
}
