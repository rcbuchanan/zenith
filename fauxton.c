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
#include "skybox.h"


#define SB_BUTTONS_COUNT 20


void display();
void reshape(int, int);
void spaceball_motion(int, int, int);
void spaceball_rotate(int, int, int);
void keyboard_key(unsigned char, int, int);
void glut_setup(int, char **);


GLuint winW = 512;
GLuint winH = 512;
int sb_buttons[SB_BUTTONS_COUNT];

struct landscape *landscape;
struct view *view;


void display()
{
	static GLfloat t;

	//glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	view_update(view);
	modelview_translate(view->eye[0], -view->eye[1], -view->eye[2]);
	axis_draw();

	modelview_pushident();
	//modelview_rotate(0, 1, 0, (t += 0.001));
	//landscape_draw(landscape);
	skybox_draw();

	modelview_pop();

	glFlush();
	//glutSwapBuffers();
}

void reshape(int w, int h)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, w, h);

	winW = w;
	winH = h;
	projection_set_perspective(90, (w * 1.f) / h, 1, 40);
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

	printf("r: %04.04f %04.04f %04.04f\n", rx * 1.f, ry * 1.f, rz * 1.f);
	modelview_rotate(rx, ry, rz, k);
}

void spaceball_button(int button, int state)
{
	printf("%d, %d\n", button, state);
	sb_buttons[button] = state;
}

void keyboard_key(unsigned char key, int x, int y)
{
	GLfloat d = 0.1;

	/* X right to left */
	/* Y down to up */
	/* Z in to out */
	switch (key) {
	  case 'a':
		  view_translate(view, 0, d, 0);
		  break;
	  case 'e':
		  view_translate(view, 0, -d, 0);
		  break;

	  case '.':
		  view_translate(view, 0, 0, d);
		  break;
	  case '\'':
		  view_translate(view, 0, 0, -d);
		  break;

	  case ',':
		  view_translate(view, d, 0, 0);
		  break;
	  case 'o':
		  view_translate(view, -d, 0, 0);
		  break;

	  case 'A':
		  view_rotate(view, 0, 1, 0, d);
		  break;
	  case 'E':
		  view_rotate(view, 0, -1, 0, d);
		  break;

	  case '>':
		  view_rotate(view, 0, 0, 1, d);
		  break;
	  case '"':
		  view_rotate(view, 0, 0, -1, d);
		  break;

	  case '<':
		  view_rotate(view, 1, 0, 0, d);
		  break;
	  case 'O':
		  view_rotate(view, -1, 0, 0, d);
		  break;
	}
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
	view = view_create(eye, obj, up);

	landscape = landscape_create();

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
