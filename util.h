
struct GLbuffer {
	void	*d;
	GLuint	 n;
	GLsizei	 size;
	GLuint	 id;
};

struct GLvarray {
	struct	GLbuffer *buf;
	GLuint	id;
};

struct GLprogram {
	GLuint	vsid;
	GLuint	fsid;
	GLuint	id;
};

struct GLunibuf {
	struct	 GLuni *unis;
	struct	 GLbuffer *buf;
	GLuint	 nuni;
	GLuint	 id;
};

struct GLuni {
	GLuint	 off;
	GLenum	 type;
	GLenum	 size;
	GLuint	 id;
};


struct	GLbuffer *create_GLbuffer(GLsizei s, GLuint n);
void	free_GLbuffer(struct GLbuffer *);

struct	GLprogram *create_GLprogram(const char *, const char *);
void	free_GLprogram(struct GLprogram *);

struct	GLunibuf *create_GLunibuf(struct GLprogram *, const char *);
void	free_GLunibuf(struct GLunibuf *);

void	print_program_log(GLuint);
void	print_shader_log(GLuint);
GLuint	load_shader(const char *, GLenum);
size_t	gl_sizeof(GLenum);
