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

#include "landscape.h"


#define SB_BUTTONS_COUNT 20


void display();
void reshape(int, int);
void spaceball_motion(int, int, int);
void spaceball_rotate(int, int, int);
void glut_setup(int, char **);

void *reload_shader_thread(void *);
void setup_reload_shader_thread(struct GLshader *);


GLuint winW = 512;
GLuint winH = 512;
int sb_buttons[SB_BUTTONS_COUNT];

struct landscape *landscape;

void display()
{

	modelview_rotate(1.f, 1.f, 1.f, 0.001f);

	//glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniformMatrix4fv(0, 1, GL_TRUE,
			   (float *) projection_modelview_collapse());
	landscape_draw(landscape);

	glFlush();
	//glutSwapBuffers();
}

void reshape(int w, int h)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, w, h);

	winW = w;
	winH = h;
	projection_set_perspective(90, (w * 1.f) / h, 1, 10);
}

void spaceball_motion(int x, int y, int z)
{
	GLfloat k = 0.0001;

	//printf("t: %04.04f %04.04f %04.04f\n", x * k, y * k, z * k);
	modelview_translate(x * k, y * k, z * k);
}

void spaceball_rotate(int rx, int ry, int rz)
{
	GLfloat k = 0.01;

	printf("r: %04.04f %04.04f %04.04f\n", rx * 1.f, ry * 1.f,
	       rz * 1.f);
	modelview_rotate(rx, ry, rz, k);
}

void spaceball_button(int button, int state)
{
	printf("%d, %d\n", button, state);
	sb_buttons[button] = state;
}


void glut_setup(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(winW, winH);
	glutInitContextVersion(3, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(argv[0]);
	glutSetCursor(GLUT_CURSOR_NONE);

	printf("%s\n", (char *) glGetString(GL_VERSION));

	if (glewInit()) {
		printf("Unable to initialize GLEW ...\n");
		exit(-1);
	}
}

int main(int argc, char **argv)
{
	vec3 eye = { 0, 0, -5 };
	vec3 obj = { 0, 0, 0 };
	vec3 up = { 0, 1, 0 };

	glut_setup(argc, argv);

	memset(sb_buttons, 0, sizeof(sb_buttons));

	projection_pushident();
	modelview_pushident();
	modelview_lookat(eye, obj, up);
	modelview_pushident();

	landscape = landscape_create();

	glutSpaceballMotionFunc(spaceball_motion);
	glutSpaceballRotateFunc(spaceball_rotate);
	glutSpaceballButtonFunc(spaceball_button);

	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(reshape);
	glutMainLoop();

	return 0;
}
