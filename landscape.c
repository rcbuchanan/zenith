#include <err.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "linmath/linmath.h"

#include "util.h"

#include "filewatcher.h"
#include "matstack.h"
#include "landscape.h"
#include "view.h"
#include "polyhedron.h"


static struct GLprogram line_prog;
static struct GLprogram shade_prog;
static struct watched_program line_watch;
static struct watched_program shade_watch;
static int init_done = 0;

static struct GLvarray vtx;
static struct GLbuffer tri;


static void init_programs();
static void init_surface();


static void init_programs(void)
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

	create_watched_program(&line_watch, &shade_prog);
	create_watched_program(&shade_watch, &line_prog);
}

static void init_surface(void)
{
	struct polyhedron p, pp;
	int i;
	GLfloat(*fp)[6];
	GLuint(*up)[3];

	polyhedron_subdivide(&p, &ico_polyhedron);
	for (i = 0; i < 4; i++) {
		polyhedron_subdivide(&pp, &p);
		polyhedron_free(&p);
		p = pp;
	}

	create_GLvarray(&vtx, sizeof(GLfloat), 6 * p.nv);
	if ((fp = malloc(vtx.buf.size)) == NULL)
		errx(1, __FILE__ ": malloc");

	for (i = 0; i < p.nv; i++) {
		fp[i][0] = p.v[i][0];
		fp[i][1] = p.v[i][1];
		fp[i][2] = p.v[i][2];
		fp[i][3] = p.v[i][0];
		fp[i][4] = p.v[i][1];
		fp[i][5] = p.v[i][2];
	}

	bindonce_GLbuffer(&vtx.buf, GL_ARRAY_BUFFER, fp);
	free(fp);

	create_GLbuffer(&tri, sizeof(GLuint), 3 * p.nf);
	if ((up = malloc(tri.size)) == NULL)
		errx(1, __FILE__ ": malloc");

	for (i = 0; i < p.nf; i++) {
		up[i][0] = (GLfloat) p.f[i][0];
		up[i][1] = (GLfloat) p.f[i][1];
		up[i][2] = (GLfloat) p.f[i][2];
	}

	bindonce_GLbuffer(&tri, GL_ELEMENT_ARRAY_BUFFER, up);
	free(up);

	polyhedron_free(&p);
}

void landscape_draw(float t)
{
	if (!init_done) {
		init_programs();
		init_surface();
		init_done = 1;
	}

	update_program(&line_watch);
	update_program(&shade_watch);

	glBindVertexArray(vtx.id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tri.id);
	glBindBuffer(GL_ARRAY_BUFFER, vtx.buf.id);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
			      3 * 2 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * 2 * sizeof(GLfloat),
			      (void *) (sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(1);

	//glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);
	//glCullFace(GL_FRONT);

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
	glDrawElements(GL_TRIANGLES, tri.n, GL_UNSIGNED_INT, 0);

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

	glDrawElements(GL_TRIANGLES, tri.n, GL_UNSIGNED_INT, 0);
	glDisable(GL_POLYGON_OFFSET_FILL);
}
