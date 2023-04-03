#include <err.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "linmath/linmath.h"

#include "geom.h"
#include "polyhedron.h"


struct hyperedge {
	GLuint v0;
	GLuint v1;
	int index;
};

struct hypernode {
	GLuint(*f)[3];
	struct hyperedge *e[3];
};

static const int const match[9][4] = {
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

struct polyhedron ico_polyhedron = {
	.v = ico_vtx,
	.nv = 12,
	.f = ico_faces,
	.nf = 20,
};

static void update_adj(struct hypernode *hna, struct hypernode *hnb,
		       struct hyperedge **hep);


static void update_adj(struct hypernode *hna, struct hypernode *hnb,
		       struct hyperedge **hep)
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

void polyhedron_subdivide(struct polyhedron *out,
			  struct polyhedron *in)
{
	struct hyperedge *he;
	struct hypernode *hn;
	struct hyperedge *hep;

	int nhe;
	int nhn;

	GLuint i;
	GLuint j;

	//GLfloat area;
	//vec3 tmp;

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
	for (i = 0; i < in->nv; i++)
		vec3_norm(out->v[i], in->v[i]);
	//for (i = 0; i < in->nv; i++)
	//vec3_dup(out->v[i], in->v[i]);
	//	vec3_sub(tmp, out->v[he[0].v0], out->v[he[0].v1]);
	//	area = vec3_len(tmp);

	// create subdivided verticies (normalized)
	for (i = 0; i < nhe; i++) {
		he[i].index = nhn + i;
		vec3_add(out->v[nhn + i], out->v[he[i].v0], out->v[he[i].v1]);
		vec3_scale(out->v[nhn + i], out->v[nhn + i], 0.5);
		vec3_norm(out->v[nhn + i], out->v[nhn + i]);
		//vec3_scale(out->v[nhn + i], out->v[nhn + i],
		//	   1.f + (random() * .4f / RAND_MAX) * area);
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

void polyhedron_free(struct polyhedron *p)
{
	if (p->f)
		free(p->f);
	if (p->v)
		free(p->v);
}
