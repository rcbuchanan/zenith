#include <err.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <sys/inotify.h>

#include "util.h"

#include "linmath.h"
#include "matstack.h"
#include "view.h"

#include "landscape.h"


#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define SB_BUTTONS_COUNT 20


void	display();
void	reshape(int, int);
void	spaceball_motion(int, int, int);
void	spaceball_rotate(int, int, int);
void	glut_setup(int, char **);

void	*reload_shader_thread(void *);
void	 setup_reload_shader_thread(struct GLshader *);


GLuint	winW = 512;
GLuint	winH = 512;
int	sb_buttons[SB_BUTTONS_COUNT];

struct	GLprogram *program;
struct	landscape *landscape;

pthread_mutex_t	reprogram_mtx;
int		reprogram = 0;


void
display()
{
	struct	GLprogram *pp;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program->id);

	glUniformMatrix4fv(0, 1, GL_TRUE, (float *) projection_modelview_collapse());
	landscape_draw(landscape);

	if (reprogram && pthread_mutex_trylock(&reprogram_mtx) == 0) {
		printf("YAY!\n");
		pp = create_GLprogram("./s.vert", "./s.frag");
		free_GLprogram(program);
		program = pp;

		reprogram = 0;
		if (pthread_mutex_unlock(&reprogram_mtx))
			errx(1, __FILE__ ": problem with pthreads");
	}

	glFlush();
	//glutSwapBuffers();
}

void
reshape(int w, int h)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, w, h);

	winW = w; winH = h;
	projection_set_perspective(90, (w * 1.f) / h, 0, 10);
}

void
spaceball_motion(int x, int y, int z)
{
	GLfloat k = 0.0001;

	if (sb_buttons[7]) return;

	//printf("t: %04.04f %04.04f %04.04f\n", x * k, y * k, z * k);
	//modelview_translate(x * k, y * k, z * k);
}

void
spaceball_rotate(int rx, int ry, int rz)
{
	GLfloat k = 0.01;

	if (sb_buttons[6]) return;

	printf("r: %04.04f %04.04f %04.04f\n", rx * 1.f, ry * 1.f, rz * 1.f);
	modelview_rotate(rx, ry, rz, k);
}

void
spaceball_button(int button, int state)
{
	printf("%d, %d\n", button, state);
	sb_buttons[button] = state;
}


void
glut_setup(int argc, char **argv)
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

void *
reload_shader_thread(void *v)
{
	struct	inotify_event ev;
	char	buf[256];
	struct	GLshader *s;
	int	fd;
	int	n;

	s = (struct GLshader *) v;
	if ((fd = inotify_init()) == -1)
		errx(1, __FILE__ ": auto reloading shader stuff");
		
	if (inotify_add_watch(fd, s->path, IN_MODIFY) == -1)
		errx(1, __FILE__ ": auto reloading shader stuff");

	for(;;) {
		n = read(fd, &ev, sizeof (struct inotify_event)); 
		if (n <= 0)	break;

		while (ev.len > 0) {
			n = read(fd, &buf, MIN(sizeof (buf), ev.len));
			if (n <= 0)	break;
			ev.len -= n;
		}

		printf("update\n");
		if (pthread_mutex_lock(&reprogram_mtx))
			errx(1, __FILE__ ": problem with pthreads");
		printf("locked\n");
		reprogram = 1;
		if (pthread_mutex_unlock(&reprogram_mtx))
			errx(1, __FILE__ ": problem with pthreads");
		printf("unlocked\n");

	}

	errx(1, ": quiting auto reloading shader");
	return NULL;
}

void
setup_reload_shader_thread(struct GLshader *s)
{
	pthread_t	id;
	pthread_attr_t	attr;

	pthread_mutex_init(&reprogram_mtx, NULL);


	if (pthread_attr_init(&attr))
		errx(1, __FILE__ ": no pthread");

	if (pthread_create(&id, &attr, *reload_shader_thread, s))
		errx(1, __FILE__ ": no pthread");
}


int
main(int argc, char **argv)
{
	vec3	eye = {0, 0, -5};
	vec3	obj = {0, 0, 0};
	vec3	up = {0, 1, 0};

	glut_setup(argc, argv);

	memset(sb_buttons, 0, sizeof (sb_buttons));

	program = create_GLprogram("./s.vert", "./s.frag");
	if (program == NULL)
		errx(1, __FILE__ ": no program.");

	setup_reload_shader_thread(program->fs);

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
