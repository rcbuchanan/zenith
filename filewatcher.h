struct watched_program;

void	update_program(struct watched_program *);
struct	watched_program *create_watched_program(struct GLprogram *);
void	free_watched_program(struct watched_program *);
