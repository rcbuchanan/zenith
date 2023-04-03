struct polyhedron {
	GLfloat(*v)[3];
	int nv;
	GLuint(*f)[3];
	int nf;
};


extern struct polyhedron ico_polyhedron;


void polyhedron_subdivide(struct polyhedron *, struct polyhedron *);
void polyhedron_free(struct polyhedron *);
