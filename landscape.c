#include <err.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <gsl/gsl_math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include <GL/glew.h>

#include "linmath/linmath.h"

#include "util.h"

#include "filewatcher.h"
#include "matstack.h"
#include "view.h"


struct hyperedge {
	GLuint v0;
	GLuint v1;
	int index;
};

struct hypernode {
	GLuint(*f)[3];
	struct hyperedge *e[3];
};

struct landscape {
	struct GLvarray vtx;
	struct GLbuffer tri;
};

struct polyhedron {
	GLfloat(*v)[3];
	int nv;
	GLuint(*f)[3];
	int nf;
};


static struct GLprogram line_prog;
static struct GLprogram shade_prog;
static struct watched_program shwatch;
static struct watched_program shwatch2;
static int init_done = 0;

static int match[9][4] = {
	{0, 1, 0, 2},
	{1, 2, 0, 2},
	{2, 0, 0, 2},
	{0, 1, 1, 0},
	{1, 2, 1, 0},
	{2, 0, 1, 0},
	{0, 1, 2, 1},
	{1, 2, 2, 1},
	{2, 0, 2, 1}
};

static GLfloat ico_vtx[12][3] = {
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

static GLuint ico_faces[20][3] = {
	{1,   2,  0},
	{2,   3,  0},
	{3,   4,  0},
	{4,   5,  0},
	{5,   1,  0},
	{10,  6,  1},
	{6,   2,  1},
	{6,   7,  2},
	{7,   3,  2},
	{7,   8,  3},
	{8,   4,  3},
	{8,   9,  4},
	{9,   5,  4},
	{9,  10,  5},
	{10,  1,  5},
	{11,  6, 10},
	{11,  7,  6},
	{11,  8,  7},
	{11,  9,  8},
	{11, 10,  9}
};

static struct polyhedron ico_polyhedron = {
	.v = ico_vtx,
	.nv = 12,
	.f = ico_faces,
	.nf = 20,
};


static void create_programs();
static void polyhedron_createsubdivided(struct polyhedron *,
					struct polyhedron *);
static void polyhedron_free(struct polyhedron *);


void create_programs()
{
	struct GLshader *vtx;

	if ((vtx = malloc(sizeof (struct GLshader))) == NULL)
		errx(1, __FILE__ ": malloc");
	create_GLshader(vtx, "./s.vert", GL_VERTEX_SHADER);

	create_GLprogram(&line_prog);
	addshader_GLprogram(&line_prog, vtx);
	addnewshader_GLprogram(&line_prog, "./line.frag", GL_FRAGMENT_SHADER);
	link_GLprogram(&line_prog);

	create_GLprogram(&shade_prog);
	addshader_GLprogram(&shade_prog, vtx);
	addnewshader_GLprogram(&shade_prog, "./shade.frag", GL_FRAGMENT_SHADER);
	link_GLprogram(&shade_prog);

	create_watched_program(&shwatch, &shade_prog);
	create_watched_program(&shwatch2, &line_prog);

	init_done = 1;
}

void landscape_create(struct landscape *l)
{
	struct polyhedron p, pp;
	int i;
	GLfloat(*fp)[6];
	GLuint(*up)[3];

	if (!init_done)
		create_programs();

	polyhedron_createsubdivided(&p, &ico_polyhedron);
	for (i = 0; i < 4; i++) {
		polyhedron_createsubdivided(&pp, &p);
		polyhedron_free(&p);
		p = pp;
	}

	create_GLvarray(&l->vtx, sizeof(GLfloat), 6 * p.nv);
	if ((fp = malloc(l->vtx.buf.size)) == NULL)
		errx(1, __FILE__ ": malloc");

	for (i = 0; i < p.nv; i++) {
		fp[i][0] = p.v[i][0];
		fp[i][1] = p.v[i][1];
		fp[i][2] = p.v[i][2];
		fp[i][3] = p.v[i][0];
		fp[i][4] = p.v[i][1];
		fp[i][5] = p.v[i][2];
	}

	bindonce_GLbuffer(&l->vtx.buf, GL_ARRAY_BUFFER, fp);
	free(fp);

	create_GLbuffer(&l->tri, sizeof(GLuint), 3 * p.nf);
	if ((up = malloc(l->tri.size)) == NULL)
		errx(1, __FILE__ ": malloc");

	for (i = 0; i < p.nf; i++) {
		up[i][0] = (GLfloat) p.f[i][0];
		up[i][1] = (GLfloat) p.f[i][1];
		up[i][2] = (GLfloat) p.f[i][2];
	}

	bindonce_GLbuffer(&l->tri, GL_ELEMENT_ARRAY_BUFFER, up);
	free(up);

	polyhedron_free(&p);
}

void landscape_draw(struct landscape *l)
{
	static float t = 0;
	update_program(&shwatch);
	update_program(&shwatch2);

	glBindVertexArray(l->vtx.id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, l->tri.id);
	glBindBuffer(GL_ARRAY_BUFFER, l->vtx.buf.id);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
			      3 * 2 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * 2 * sizeof(GLfloat),
			      (void *) (sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(1);

	glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);
	glCullFace(GL_FRONT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glUseProgram(line_prog.id);
	glUniformMatrix4fv(0, 1, GL_FALSE,
			   (float *) modelview_collapse());
	glUniformMatrix4fv(1, 1, GL_FALSE,
			   (float *) projection_collapse());
	glUniform1f(2, t += 0.001);
	glUniform1f(3, 0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(1);
	glDrawElements(GL_TRIANGLES, l->tri.n, GL_UNSIGNED_INT, 0);

	glCullFace(GL_BACK);

	glUseProgram(shade_prog.id);
	glUniformMatrix4fv(0, 1, GL_FALSE,
			   (float *) modelview_collapse());
	glUniformMatrix4fv(1, 1, GL_FALSE,
			   (float *) projection_collapse());
	glUniform1f(2, t);
	glUniform1f(3, 0);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPolygonOffset(1.0, 1.0);

	glDrawElements(GL_TRIANGLES, l->tri.n, GL_UNSIGNED_INT, 0);
	glDisable(GL_POLYGON_OFFSET_FILL);
}

static void
update_adj(struct hypernode *hna, struct hypernode *hnb, struct hyperedge **hep)
{
	GLuint *fa;
	GLuint *fb;
	int i;

	fa = *hna->f;
	fb = *hnb->f;

	for (i = 0; i < 9; i++) {
		if (fa[match[i][0]] != fb[match[i][2]])
			continue;
		if (fa[match[i][1]] != fb[match[i][3]])
			continue;

		(*hep)->v0 = fa[match[i][0]];
		(*hep)->v1 = fa[match[i][1]];

		hna->e[match[i][0]] = *hep;
		hnb->e[match[i][3]] = *hep;

		(*hep)++;
	}
}

static void polyhedron_createsubdivided(struct polyhedron *out,
					struct polyhedron *in)
{
	struct hyperedge *he;
	struct hypernode *hn;
	struct hyperedge *hep;

	int nhe;
	int nhn;

	GLuint i;
	GLuint j;

	GLfloat area;
	vec3 tmp;

	nhe = in->nf * 3 / 2;
	nhn = in->nf;

	if ((he = malloc(sizeof(he[0]) * nhe)) == NULL)
		errx(1, __FILE__ ": malloc");
	if ((hn = malloc(sizeof(hn[0]) * nhn)) == NULL)
		errx(1, __FILE__ ": malloc");

	// nodes
	memset(hn, 0, sizeof(hn[0]) * nhn);
	for (i = 0; i < nhn; i++)
		hn[i].f = in->f + i;

	// edges
	for (hep = he, i = 0; i < nhn; i++)
		for (j = i + 1; j < nhn; j++)
			update_adj(hn + i, hn + j, &hep);

	out->nv = nhn + nhe;
	out->nf = nhn * 4;
	if ((out->v = malloc(sizeof(out->v[0]) * out->nv)) == NULL)
		errx(1, __FILE__ ": malloc");
	if ((out->f = malloc(sizeof(out->f[0]) * out->nf)) == NULL)
		errx(1, __FILE__ ": malloc");

	// copy original verticies (normalized)
	//for (i = 0; i < in->nv; i++)
	//	vec3_norm(out->v[i], in->v[i]);
	for (i = 0; i < in->nv; i++)
		vec3_dup(out->v[i], in->v[i]);
	vec3_sub(tmp, out->v[he[0].v0], out->v[he[0].v1]);
	area = vec3_len(tmp);

	// create subdivided verticies (normalized)
	for (i = 0; i < nhe; i++) {
		he[i].index = nhn + i;
		vec3_add(out->v[nhn + i], out->v[he[i].v0], out->v[he[i].v1]);
		vec3_scale(out->v[nhn + i], out->v[nhn + i], 0.5);
		//vec3_norm(out->v[nhn + i], out->v[nhn + i]);
		vec3_scale(out->v[nhn + i], out->v[nhn + i],
			   1.f + (random() * .4f / RAND_MAX) * area);
	}

	// create new faces
	for (i = 0; i < nhn; i++) {
		out->f[4 * i + 0][0] = (*hn[i].f)[0];
		out->f[4 * i + 0][1] = hn[i].e[0]->index;
		out->f[4 * i + 0][2] = hn[i].e[2]->index;

		out->f[4 * i + 1][0] = hn[i].e[0]->index;
		out->f[4 * i + 1][1] = hn[i].e[1]->index;
		out->f[4 * i + 1][2] = hn[i].e[2]->index;

		out->f[4 * i + 2][0] = (*hn[i].f)[1];
		out->f[4 * i + 2][1] = hn[i].e[1]->index;
		out->f[4 * i + 2][2] = hn[i].e[0]->index;

		out->f[4 * i + 3][0] = (*hn[i].f)[2];
		out->f[4 * i + 3][1] = hn[i].e[2]->index;
		out->f[4 * i + 3][2] = hn[i].e[1]->index;
	}

	free(he);
	free(hn);
}

static void polyhedron_free(struct polyhedron *p)
{
	if (p->f)
		free(p->f);
	if (p->v)
		free(p->v);
}
