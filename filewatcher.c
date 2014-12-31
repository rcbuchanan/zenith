#include <err.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <GL/gl.h>

#include <sys/inotify.h>

#include "util.h"


#define MIN(x, y) (((x) < (y)) ? (x) : (y))


struct watched_program {
	struct GLprogram *p;
	int needs_update;
	pthread_mutex_t m;
};


static void *watched_program_thread(void *);


void create_watched_program(struct watched_program *w, struct GLprogram *p)
{
	pthread_t id;
	pthread_attr_t attr;

	w->p = p;
	pthread_mutex_init(&w->m, NULL);
	w->needs_update = 0;

	if (pthread_attr_init(&attr))
		errx(1, __FILE__ ": no pthread");
	if (pthread_create(&id, &attr, *watched_program_thread, w))
		errx(1, __FILE__ ": no pthread");
}

void update_program(struct watched_program *w)
{
	int i;
	if (w->needs_update && !pthread_mutex_trylock(&w->m)) {
		for (i = 0; i < w->p->ns; i++)
			load_GLshader(w->p->ss[i]);
		link_GLprogram(w->p);

		w->needs_update = 0;
		if (pthread_mutex_unlock(&w->m))
			errx(1, __FILE__ ": problem with pthreads");
	}
}

void free_watched_program(struct watched_program *w)
{
	pthread_mutex_destroy(&w->m);
}

static void *watched_program_thread(void *v)
{
	struct inotify_event ev;
	char buf[256];
	struct watched_program *w;
	int fd;
	int n;
	int i;

	w = (struct watched_program *) v;
	if ((fd = inotify_init()) == -1)
		errx(1, __FILE__ ": auto reloading shader stuff");

	for (i = 0; i < w->p->ns; i++)
		if (inotify_add_watch(fd, w->p->ss[i]->path, IN_MODIFY) == -1)
			errx(1, __FILE__ ": auto reloading shader stuff");

	for (;;) {
		n = read(fd, &ev, sizeof(struct inotify_event));
		if (n <= 0)
			break;

		while (ev.len > 0) {
			n = read(fd, &buf, MIN(sizeof(buf), ev.len));
			if (n <= 0)
				break;
			ev.len -= n;
		}

		printf("update\n");
		if (pthread_mutex_lock(&w->m))
			errx(1, __FILE__ ": problem with pthreads");
		printf("locked\n");
		w->needs_update = 1;
		if (pthread_mutex_unlock(&w->m))
			errx(1, __FILE__ ": problem with pthreads");
		printf("unlocked\n");

	}

	errx(1, ": quiting auto reloading shader");
	return NULL;
}
