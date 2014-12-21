GLfloat *modelview_collapse();
void modelview_loadident();
void modelview_pushident();
void modelview_pop();
void modelview_lookat(vec3, vec3, vec3);
void modelview_translate(GLfloat, GLfloat, GLfloat);
void modelview_rotate(GLfloat, GLfloat, GLfloat, GLfloat);

GLfloat *projection_collapse();
void projection_loadident();
void projection_pushident();
void projection_pop();
void projection_set_perspective(GLfloat, GLfloat, GLfloat, GLfloat);

GLfloat *projection_modelview_collapse();
