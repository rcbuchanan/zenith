struct watched_program {
	struct GLprogram *p;
	int needs_update;
	pthread_mutex_t m;
};


void create_watched_program(struct watched_program *, struct GLprogram *);
void update_program(struct watched_program *);
void free_watched_program(struct watched_program *);
