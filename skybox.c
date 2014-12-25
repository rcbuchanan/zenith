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


static struct GLvarray *vdata = NULL;
static struct GLbuffer *fdata = NULL;
static struct GLprogram *vprog = NULL;
static struct GLtexture *tdata = NULL;
static struct GLtexture *tdata2 = NULL;


static char *cube_tga[6] = {
	"skypics/posx.tga",
	"skypics/negy.tga",
	"skypics/posz.tga",
	"skypics/negx.tga",
	"skypics/posy.tga",
	"skypics/negz.tga"
};

static char *cube2_tga[6] = {
	"powerpics/posx.tga",
	"powerpics/negy.tga",
	"powerpics/posz.tga",
	"powerpics/negx.tga",
	"powerpics/posy.tga",
	"powerpics/negz.tga"
};

static GLfloat cube_vtx[8 * 3] = {
	1.000000 , -1.000000, -1.000000,
	1.000000 , -1.000000,  1.000000,
	-1.000000, -1.000000,  1.000000,
	-1.000000, -1.000000, -1.000000,
	1.000000 ,  1.000000, -1.000000,
	1.000000 ,  1.000000,  1.000000,
	-1.000000,  1.000000,  1.000000,
	-1.000000,  1.000000, -1.000000
};
static GLuint cube_faces[12 * 3] = {
	0, 1, 2,
	4, 7, 6,
	0, 4, 5,
	1, 5, 6,
	2, 6, 7,
	4, 0, 3,

	0, 2, 3,
	4, 6, 5,
	0, 5, 1,
	1, 6, 2,
	2, 7, 3,
	4, 3, 7,
};


static void skybox_init()
{
	struct GLshader *vtx;
	struct GLshader *line;

	vtx = create_GLshader("./skybox.vert", GL_VERTEX_SHADER);
	line = create_GLshader("./skybox.frag", GL_FRAGMENT_SHADER);

	vprog = create_GLprogram();
	addshader_GLprogram(vprog, vtx);
	addshader_GLprogram(vprog, line);
	link_GLprogram(vprog);

	vdata = create_GLvarray(sizeof(GLfloat), ARRAY_SIZE(cube_vtx));
	bindonce_GLbuffer(vdata->buf, GL_ARRAY_BUFFER, cube_vtx);

	fdata = create_GLbuffer(sizeof(GLfloat), ARRAY_SIZE(cube_faces));
	bindonce_GLbuffer(fdata, GL_ELEMENT_ARRAY_BUFFER, cube_faces);

	glActiveTexture(GL_TEXTURE0);
	tdata = create_GLtexture(2048, 2048);
	loadtgacube_GLtexture(tdata, cube_tga);

	glActiveTexture(GL_TEXTURE1);
	tdata2 = create_GLtexture(1024, 1024);
	loadtgacube_GLtexture(tdata2, cube2_tga);

}

void skybox_draw()
{
	if (vprog == NULL)
		skybox_init();

	glBindVertexArray(vdata->id);
	glBindBuffer(GL_ARRAY_BUFFER, vdata->buf->id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fdata->id);

	glVertexAttribPointer(0,
			      3,
			      GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);

	glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);
	glCullFace(GL_FRONT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glUseProgram(vprog->id);
	glUniformMatrix4fv(0, 1, GL_TRUE,
			   (float *) projection_modelview_collapse());
	glUniform1i(1, 0);
	glUniform1i(2, 1);

	glDrawElements(GL_TRIANGLES, fdata->n, GL_UNSIGNED_INT, 0);
}
