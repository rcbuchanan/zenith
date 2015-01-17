struct view {
	vec3 eye;
	vec3 obj;
	vec3 up;
};


void view_create(struct view *, vec3, vec3, vec3);
void view_rotate(struct view *, GLfloat, GLfloat, GLfloat, GLfloat);
void view_translate(struct view *, GLfloat, GLfloat, GLfloat);
void view_update(struct view *);
