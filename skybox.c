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
static struct GLbuffer fdata;
static struct GLprogram vprog;
static struct GLtexture tdata;
static struct GLtexture tdata2;
static int init_done = 0;

static struct watched_program pwatch;


static char *cube_files[6] = {
	"skypics/posx.tga",
	"skypics/negx.tga",
	"skypics/negy.tga",
	"skypics/posy.tga",
	"skypics/posz.tga",
	"skypics/negz.tga"
};

static char *cube2_files[6] = {
	"powerpics/posx.tga",
	"powerpics/negx.tga",
	"powerpics/negy.tga",
	"powerpics/posy.tga",
	"powerpics/posz.tga",
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
	create_GLprogram(&vprog);
	addnewshader_GLprogram(&vprog, "./skybox.vert", GL_VERTEX_SHADER);
	addnewshader_GLprogram(&vprog, "./skybox.frag", GL_FRAGMENT_SHADER);
	link_GLprogram(&vprog);

	create_GLvarray(&vdata, sizeof(GLfloat), ARRAY_SIZE(cube_vtx));
	bindonce_GLbuffer(&vdata.buf, GL_ARRAY_BUFFER, cube_vtx);

	create_GLbuffer(&fdata, sizeof(GLfloat), ARRAY_SIZE(cube_faces));
	bindonce_GLbuffer(&fdata, GL_ELEMENT_ARRAY_BUFFER, cube_faces);

	glActiveTexture(GL_TEXTURE0);
	create_GLtexture(&tdata, 2048, 2048);
	loadtgacube_GLtexture(&tdata, cube_files);

	glActiveTexture(GL_TEXTURE1);
	create_GLtexture(&tdata2, 1024, 1024);
	loadtgacube_GLtexture(&tdata2, cube2_files);

	create_watched_program(&pwatch, &vprog);

	init_done = 1;
}

extern struct view *view;
void skybox_draw()
{
	if (!init_done)
		skybox_init();

	update_program(&pwatch);

	glBindVertexArray(vdata.id);
	glBindBuffer(GL_ARRAY_BUFFER, vdata.buf.id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fdata.id);

	glVertexAttribPointer(0,
			      3,
			      GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);

	//glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);
	//glCullFace(GL_FRONT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glUseProgram(vprog.id);
	glUniformMatrix4fv(0, 1, GL_FALSE,
			   (float *) modelview_collapse());
	glUniformMatrix4fv(1, 1, GL_FALSE,
			   (float *) projection_collapse());
	glUniform3f(2, view->eye[0], view->eye[1], view->eye[2]);
	glUniform1i(3, 0);
	glUniform1i(4, 1);

	glDrawElements(GL_TRIANGLES, fdata.n, GL_UNSIGNED_INT, 0);
}
