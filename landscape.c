#include <err.h>
#include <stdlib.h>
#include <string.h>

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


struct hyperedge {
	GLfloat	p[3];
};

struct hypernode {
	GLuint	(*f)[3];
	struct	 hyperedge *he[3];
};

struct landscape {
	struct	GLvarray *vtx;
	struct	GLbuffer *tri;
};

struct polyhedron {
	GLfloat	(*p)[3];
	int	 np;
	GLuint	(*f)[3];
	int	 nf;
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
	{ 1,  2,  0},
	{ 2,  3,  0},
	{ 3,  4,  0},
	{ 4,  5,  0},
	{ 5,  1,  0},
	{10,  6,  1},
	{ 6,  2,  1},
	{ 6,  7,  2},
	{ 7,  3,  2},
	{ 7,  8,  3},
	{ 8,  4,  3},
	{ 8,  9,  4},
	{ 9,  5,  4},
	{ 9, 10,  5},
	{10,  1,  5},
	{11,  6, 10},
	{11,  7,  6},
	{11,  8,  7},
	{11,  9,  8},
	{11, 10,  9}
};

static	struct polyhedron ico_polyhedron = {
	.p	= ico_points,
	.np	= 12,
	.f	= ico_faces,
	.nf	= 20,
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
	GLfloat	(*fp)[6];
	GLuint	(*up)[3];

	if (line_prog == NULL || shade_prog == NULL)
		create_programs();

	if ((l = malloc(sizeof (struct landscape))) == NULL)
		errx(1, __FILE__ ": allocation failed");

	if ((p = polyhedron_createsubdivided(&ico_polyhedron)) == NULL)
		errx(1, __FILE__ ": failed to create polyhedron");

	l->vtx = create_GLvarray(sizeof (GLfloat), 6 * p->np);
	for (fp = l->vtx->buf->d, i = 0; i < p->np; i++) {
		fp[i][0] = p->p[i][0];
		fp[i][1] = p->p[i][1];
		fp[i][2] = p->p[i][2];
		fp[i][3] = p->p[i][0];
		fp[i][4] = p->p[i][1];
		fp[i][5] = p->p[i][2];
	}
	glBindBuffer(GL_ARRAY_BUFFER, l->vtx->buf->id);
	glBufferData(
		GL_ARRAY_BUFFER,
		l->vtx->buf->size,
		l->vtx->buf->d,
		GL_STATIC_DRAW);

	l->tri = create_GLbuffer(sizeof (GLuint), 3 * p->nf);
	for (up = l->tri->d, i = 0; i < p->nf; i++) {
		up[i][0] = (GLfloat) p->f[i][0];
		up[i][1] = (GLfloat) p->f[i][1];
		up[i][2] = (GLfloat) p->f[i][2];
	}
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

	//glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);
	glCullFace(GL_FRONT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glUseProgram(line_prog->id);
	glUniformMatrix4fv(0, 1, GL_TRUE, (float *) projection_modelview_collapse());

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(3);
	glDrawElements(GL_TRIANGLES, l->tri->n * 3 / 4 + 3 * 5, GL_UNSIGNED_INT, 0);

	glCullFace(GL_BACK);

	glUseProgram(shade_prog->id);
	glUniformMatrix4fv(0, 1, GL_TRUE, (float *) projection_modelview_collapse());

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPolygonOffset(1.0, 1.0);

	glDrawElements(GL_TRIANGLES, l->tri->n * 3 / 4 + 3 * 5, GL_UNSIGNED_INT, 0);
	glDisable(GL_POLYGON_OFFSET_FILL);
}

static void
update_adj(struct hypernode *hna, struct hypernode *hnb, struct hyperedge **hep)
{
	GLuint	*fa;
	GLuint	*fb;

	fa = *hna.f;
	fb = *hnb.f;
}

static struct polyhedron *
polyhedron_createsubdivided(struct polyhedron *in)
{
	struct	polyhedron *out;

	struct	hyperedge *he;
	int	nhe;
	struct	hypernode *hn;
	int	nhn;
	struct	hyperedge *hep;

	GLuint	i;
	GLuint	j;
	
	nhe	= in->nf * 3 / 2;
	nhn	= in->nf;
	
	if ((he = malloc(sizeof (he[0]) * nhe)) == NULL)
		errx(1, __FILE__ ": malloc");
	if ((hn = malloc(sizeof (hn[0]) * nhn)) == NULL)
		errx(1, __FILE__ ": malloc");

	// nodes
	memset(hn, 0, sizeof (hn[0]) * nhn);
	for (i = 0; i < nhn; i++)
		hn[i].f = in->f + i;

	// edges
	memset(he, 0, sizeof (he[0]) * nhe);
	for (hep = he, i = 0; i < nhn; i++)
		for (j = i + 1; j < nhn; j++)
			update_adj(hn + i, hn + j, &hep);

	if ((out = malloc(sizeof (out[0]))) == NULL)
		errx(1, __FILE__ ": malloc");

	out->np = nhn + nhe;
	out->nf = nhn * 4;
	if ((out->p = malloc(sizeof (out->p[0]) * out->np)) == NULL)
		errx(1, __FILE__ ": malloc");
	if ((out->f = malloc(sizeof (out->f[0]) * out->nf)) == NULL)
		errx(1, __FILE__ ": malloc");

	// copy original points verbatim
	memcpy(out->p, in->p, sizeof (in->p[0]) * in->np);

	// copy subdivided points
	for (i = 0; i < nhe; i++)
		memcpy(out->p[nhn + i], he[i].p, sizeof (he[0].p));

	// create new faces
	for (i = 0; i < out->nf; i++)
		memset(out->f[i], 0, sizeof (out->f[0]));

	free (he);
	free (hn);

	return out;
}

static void
polyhedron_free(struct polyhedron *p)
{
	if (p->f)	free(p->f);
	if (p->p)	free(p->p);
	free(p);
}
