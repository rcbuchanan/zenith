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

#include "camera.h"
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
struct	camera *camera;
struct	landscape *landscape;

int	reprogram = 0;


void
display()
{
	struct	GLprogram *pp;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program->id);

	glUniformMatrix4fv(0, 1, GL_TRUE, (float *) camera_matrix(camera));
	landscape_draw(landscape);

	if (reprogram) {
		pp = create_GLprogram("./s.vert", "./s.frag");
		free_GLprogram(program);
		program = pp;
		reprogram = 0;
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
	camera_set_projection(camera, (w * 1.f) / h);
}

void
spaceball_motion(int x, int y, int z)
{
	GLfloat k = 0.0001;

	if (sb_buttons[7]) return;

	printf("r: %04.04f %04.04f %04.04f\n", x * k, y * k, z * k);
	camera_translate(camera, x * k, y * k, z * k);
}

void
spaceball_rotate(int rx, int ry, int rz)
{
	GLfloat k = 0.001;

	if (sb_buttons[6]) return;

	printf("r: %04.04f %04.04f %04.04f\n", rx * k, ry * k, rz * k);
	camera_rotate(camera, rx * k, ry * k, rz * k);
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
		err(1, __FILE__ ": auto reloading shader stuff");
		
	if (inotify_add_watch(fd, s->path, IN_MODIFY) == -1)
		err(1, __FILE__ ": auto reloading shader stuff");

	for(;;) {
		n = read(fd, &ev, sizeof (struct inotify_event)); 
		if (n <= 0)	break;

		while (ev.len > 0) {
			n = read(fd, &buf, MIN(sizeof (buf), ev.len));
			if (n <= 0)	break;
			ev.len -= n;
		}
		reprogram = 1;		
	}

	errx(1, ": quiting auto reloading shader");
	return NULL;
}

void
setup_reload_shader_thread(struct GLshader *s)
{
	pthread_t	id;
	pthread_attr_t	attr;

	if (pthread_attr_init(&attr))
		errx(1, __FILE__ ": no pthread");

	if (pthread_create(&id, &attr, *reload_shader_thread, s))
		errx(1, __FILE__ ": no pthread");
}


int
main(int argc, char **argv)
{
	glut_setup(argc, argv);

	memset(sb_buttons, 0, sizeof (sb_buttons));

	program = create_GLprogram("./s.vert", "./s.frag");
	if (program == NULL)
		errx(1, __FILE__ ": no program.");

	setup_reload_shader_thread(program->fs);

	camera = camera_create();
	if (camera == NULL)
		errx(1, __FILE__ ": no camera.\n");

	landscape = landscape_create();
	if (camera == NULL)
		errx(1, __FILE__ ": no landscape.\n");

	glutSpaceballMotionFunc(spaceball_motion);
	glutSpaceballRotateFunc(spaceball_rotate);
	glutSpaceballButtonFunc(spaceball_button);

	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(reshape);
	glutMainLoop();

	return 0;
}
