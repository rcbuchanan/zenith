#include <err.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "util.h"

#include "linmath/linmath.h"
#include "matstack.h"
#include "view.h"

#include "axis.h"
#include "landscape.h"
#include "orb.h"
#include "skybox.h"


#define IS_PRESSED(n) (sb_buttons & (1 << (n)))



static void render_scene(GLfloat, GLuint);
static void display();
static void reshape(int, int);
static void spaceball_motion(int, int, int);
static void spaceball_rotate(int, int, int);
static void keyboard_key(unsigned char, int, int);
static void glut_setup(int, char **);
static void reset_view();

static GLenum cube_axes[] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};
static vec3 cube_vec3[] = {
	{ 1,  0,  0},
	{-1,  0,  0},
	{ 0,  1,  0},
	{ 0, -1,  0},
	{ 0,  0,  1,},
	{ 0,  0, -1},
};

GLuint winW = 512;
GLuint winH = 512;
GLuint sb_buttons = 0;

struct view view;

struct GLtexture envmap;
struct GLframebuffer envfb[6];

static void render_scene(GLfloat t, GLuint reflect)
{
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	modelview_lookat(view.eye, view.obj, view.up);
	modelview_pushident();
	modelview_rotate(0, 1, 0, t);
	skybox_draw(view.eye);
	axis_draw();
	orb_draw(t);
	modelview_translate(2.5, 0, 0);
	landscape_draw(t);
	modelview_pop();
}

static void display()
{
	static GLfloat t;
	int i;

	t += 0.008;

	//printf("view: %f %f %f\n", view.eye[0], view.eye[1], view.eye[2]);

	for (i = 0; i < 6; i++) {
		view_rotate(&view,
			    cube_vec3[i][0],
			    cube_vec3[i][1],
			    cube_vec3[i][2],
			    M_PI / 2.f);
		rendertocube_GLframebuffer(envfb + i, &envmap, cube_axes[i]);
		render_scene(t, GL_FALSE);
		view_rotate(&view,
			    cube_vec3[i][0],
			    cube_vec3[i][1],
			    cube_vec3[i][2],
			    -M_PI / 2.f);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envmap.id);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, winW, winH);
	render_scene(t, GL_TRUE);

	glutSwapBuffers();
}

static void reshape(int w, int h)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, w, h);

	winW = w;
	winH = h;
	projection_set_perspective(90, w * 1.f / h, 0.01, 100);
}

static void spaceball_motion(int x, int y, int z)
{
	GLfloat k = 0.0001;

	printf("t: %04.04f %04.04f %04.04f\n", x * 1.f, y * 1.f, -z * 1.f);
	view_translate(&view, x * k, y * k, -z * k);
}

static void spaceball_rotate(int rx, int ry, int rz)
{
	GLfloat k = M_PI / 1000.f;

	printf("r: %04.04f %04.04f %04.04f\n", rx * 1.f, ry * 1.f, -rz * 1.f);
	view_rotate(&view, rx, ry, -rz, k);
}

static void spaceball_button(int button, int state)
{
	printf("%d, %d\n", button, state);
	if(state) sb_buttons |= 1 << button;
	else sb_buttons &= ~(1 << button);

	if (button == 0 && state == 1) {
		printf("reset view\n");
		reset_view();
	}
}

static void keyboard_key(unsigned char key, int x, int y)
{
	GLfloat d = M_PI / 8.f;

	/* X right to left */
	/* Y down to up */
	/* Z in to out */
	switch (key) {
	case 'a': view_translate(&view,-d, 0, 0); break;
	case 'e': view_translate(&view, d, 0, 0); break;

	case '.': view_translate(&view, 0, 0,-d); break;
	case '\'':view_translate(&view, 0, 0, d); break;

	case ',': view_translate(&view, 0, d, 0); break;
	case 'o': view_translate(&view, 0,-d, 0); break;

	case 'A': view_rotate(&view, 0, 1, 0, d); break;
	case 'E': view_rotate(&view, 0,-1, 0, d); break;

	case '>': view_rotate(&view, 0, 0,-1, d); break;
	case '"': view_rotate(&view, 0, 0, 1, d); break;

	case '<': view_rotate(&view, 1, 0, 0, d); break;
	case 'O': view_rotate(&view,-1, 0, 0, d); break;
	}
}

static void glut_setup(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(winW, winH);
	glutInitContextVersion(3, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(argv[0]);
	glutSetCursor(GLUT_CURSOR_NONE);

	printf("%s\n", (char *) glGetString(GL_VERSION));

	if (glewInit())
		errx(1, "Unable to initialize GLEW ...\n");
}

static void reset_view()
{
	vec3 eye = {0, 0, 5};
	vec3 obj = {0, 0, 0};
	vec3 up = {0, 1, 0};

	view_create(&view, eye, obj, up);
}


int main(int argc, char **argv)
{
	int i;

	glut_setup(argc, argv);

	projection_pushident();
	modelview_pushident();
	reset_view();

	create_GLtexture(&envmap, 512, 512);
	framebuffercube_GLtexture(&envmap);

	for (i = 0; i < 6; i++)
		create_GLframebuffer(envfb + i, 512, 512, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glutSpaceballMotionFunc(spaceball_motion);
	glutSpaceballRotateFunc(spaceball_rotate);
	glutSpaceballButtonFunc(spaceball_button);
	glutKeyboardFunc(keyboard_key);

	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(reshape);
	glutMainLoop();

	return 0;
}
