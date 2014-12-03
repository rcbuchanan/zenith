#include <err.h>
#include <math.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include "util.h"

#include "linmath.h"


GLuint	make_program(const char *, const char *);
void	init(void);
void	reshape(int, int);
void	spaceball_motion(int, int, int);
void	spaceball_rotate(int, int, int);
void	display();
void	populate_vtxbuf(struct GLbuffer *);


GLuint	winW = 512;
GLuint	winH = 512;

struct	GLprogram *program;

struct	GLvarray *vtx;

GLfloat acx = 0.0;
GLfloat acy = 0.0;
GLfloat acz = 0.0;
GLfloat acrx = 0.0;
GLfloat acry = 0.0;
GLfloat acrz = 0.0;
GLfloat aspect = 1.0;

void
init()
{
	program = create_GLprogram("./s.vert", "./s.frag");
	if (program == NULL)
		errx(1, "%s: no program. Bailing.\n", __FILE__);
	glUseProgram(program->id);

	vtx = malloc(sizeof (struct GLvarray));

	vtx->buf = create_GLbuffer(sizeof (GLfloat), 3000000);
	if (vtx->buf == NULL)
		errx(1, "%s: no buffer. Bailing.\n", __FILE__);
	populate_vtxbuf(vtx->buf);

	glBindBuffer(GL_ARRAY_BUFFER, vtx->buf->id);
	glBufferData(
		GL_ARRAY_BUFFER,
		vtx->buf->size,
		vtx->buf->d,
		GL_STATIC_DRAW);

	glGenVertexArrays(1, &(vtx->id));
	glBindVertexArray(vtx->id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

void
populate_vtxbuf(struct GLbuffer *b)
{
	int	 i;
	GLfloat	*f = (GLfloat *) b->d;
	gsl_rng	*r;

	r = gsl_rng_alloc(gsl_rng_taus);

	for (i = 0; i < b->n; i++)
		f[i] = gsl_ran_gaussian(r, 1);

	gsl_rng_free(r);
}

void
display()
{
	mat4x4	a, b, c;
	vec3	eye = {acx, acy, acz}, center = {acx, acy, acz + 1}, up = {0, 1, 0};

	mat4x4_perspective(a, 3.14159 * 2 * 90.0 / 360, aspect, 1.0, 10.0);
	mat4x4_look_at(b, eye, center, up);
	mat4x4_mul(c, a, b);

	glUniformMatrix4fv(0, 1, GL_TRUE, (float *) c);

	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(vtx->id);
	glDrawArrays(GL_POINTS, 0, vtx->buf->n / 3);
	glFlush();
	//glutSwapBuffers();
}

void
reshape(int w, int h)
{
	glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT */ );

	winW = w;
	winH = h;
	glViewport(0, 0, w, h);
	aspect = (w * 1.f) / h;
}

void
spaceball_motion(int x, int y, int z)
{
	GLfloat k = 0.0001;
	acx -= x * k;
	acy += y * k;
	acz += z * k;

	printf("r: %04.04f %04.04f %04.04f p: %04.04f %04.04f %04.04f\n", acrx, acry, acrz, acx, acy, acz);
}

void
spaceball_rotate(int rx, int ry, int rz)
{
	GLfloat k = 0.001;
	acrx += rx * k;
	acry += ry * k;
	acrz += rz * k;

	printf("r: %04.04f %04.04f %04.04f p: %04.04f %04.04f %04.04f\n", acrx, acry, acrz, acx, acy, acz);
}

void
spaceball_button(int button, int state)
{
	printf("%d, %d\n", button, state);
}


int
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA /*| GLUT_DEPTH */ );
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

	init();

	glutSpaceballMotionFunc(spaceball_motion);
	glutSpaceballRotateFunc(spaceball_rotate);
	glutSpaceballButtonFunc(spaceball_button);

	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(reshape);

	glutMainLoop();

	return 0;
}
