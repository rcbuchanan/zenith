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


static struct GLvarray vdata;
static struct GLprogram vprog;
static int init_done = 0;


static GLfloat axis_vtx[] = {
	0.f, 0.f, 0.f, 1.f, 0.f, 0.f,	// red X
	1.f, 0.f, 0.f, 1.f, 0.f, 0.f,

	0.f, 0.f, 0.f, 0.f, 1.f, 0.f,	// green Y
	0.f, 1.f, 0.f, 0.f, 1.f, 0.f,

	0.f, 0.f, 0.f, 0.f, 0.f, 1.f,	// blue Z
	0.f, 0.f, 1.f, 0.f, 0.f, 1.f
};


static void axis_init()
{
	create_GLprogram(&vprog);
	addnewshader_GLprogram(&vprog, "./axis.vert", GL_VERTEX_SHADER);
	addnewshader_GLprogram(&vprog, "./axis.frag", GL_FRAGMENT_SHADER);
	link_GLprogram(&vprog);

	create_GLvarray(&vdata, sizeof(GLfloat), 36);
	bindonce_GLbuffer(&vdata.buf, GL_ARRAY_BUFFER, axis_vtx);

	init_done = 1;
}

void axis_draw()
{
	if (!init_done)
		axis_init();

	glBindVertexArray(vdata.id);
	glBindBuffer(GL_ARRAY_BUFFER, vdata.buf.id);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
			      (void *) (sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(1);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glUseProgram(vprog.id);
	glUniformMatrix4fv(glGetUniformLocation(vprog.id, "mvmat"),
			   1, GL_FALSE,
			   (float *) modelview_collapse());
	glUniformMatrix4fv(glGetUniformLocation(vprog.id, "promat"),
			   1, GL_FALSE,
			   (float *) projection_collapse());
	glLineWidth(4);
	glDrawArrays(GL_LINES, 0, vdata.buf.n);
}
