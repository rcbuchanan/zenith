#include <err.h>
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


#define ABS(x) (((x) > 0) ? (x) : (-(x)))


static struct GLvarray *vdata = NULL;
static struct GLprogram *vprog = NULL;

static GLfloat coloredverticies[] = {
	0.f, 0.f, 0.f, 1.f, 0.f, 0.f, // red X
	1.f, 0.f, 0.f, 1.f, 0.f, 0.f,

	0.f, 0.f, 0.f, 0.f, 1.f, 0.f, // green Y
	0.f, 1.f, 0.f, 0.f, 1.f, 0.f,

	0.f, 0.f, 0.f, 0.f, 0.f, 1.f, // blue Z
	0.f, 0.f, 1.f, 0.f, 0.f, 1.f
};


static void
axis_init()
{
	struct GLshader *vtx;
	struct GLshader *line;

	vtx = create_GLshader("./axis.vert", GL_VERTEX_SHADER);
	line = create_GLshader("./axis.frag", GL_FRAGMENT_SHADER);

	vprog = create_GLprogram();
	addshader_GLprogram(vprog, vtx);
	addshader_GLprogram(vprog, line);
	link_GLprogram(vprog);

	vdata = create_GLvarray(sizeof (GLfloat), 3 * 2 * 2 * 3);
	memcpy(vdata->buf->d, coloredverticies, vdata->buf->size);

	glBindBuffer(GL_ARRAY_BUFFER, vdata->buf->id);
	glBufferData(GL_ARRAY_BUFFER,
		     vdata->buf->size,
		     vdata->buf->d,
		     GL_STATIC_DRAW);
}

void
axis_draw()
{
	if (vprog == NULL) axis_init();

	glBindVertexArray(vdata->id);
	glBindBuffer(GL_ARRAY_BUFFER, vdata->buf->id);

	glVertexAttribPointer(
	    0,
	    3,
	    GL_FLOAT,
	    GL_FALSE,
	    3 * 2 * sizeof(GLfloat),
	    0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
	    1,
	    3,
	    GL_FLOAT,
	    GL_FALSE,
	    3 * 2 * sizeof(GLfloat),
	    (void *) (sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(1);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glUseProgram(vprog->id);
	glUniformMatrix4fv(0, 1, GL_TRUE,
			   (float *) projection_modelview_collapse());

	glLineWidth(4);
	glDrawArrays(GL_LINES, 0, 2 * 3 * 2 * 3);
}
