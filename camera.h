struct camera;

struct	 camera *camera_create();
void	 camera_rotate(struct camera *, GLfloat, GLfloat, GLfloat);
void	 camera_translate(struct camera *, GLfloat, GLfloat, GLfloat);
void	 camera_set_projection(struct camera *, GLfloat);
void	 camera_reset_modelview(struct camera *);
GLfloat	*camera_matrix(struct camera *);
