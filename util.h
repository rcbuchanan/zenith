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

struct GLshader {
	GLuint	 type;
	GLchar	*path;
	GLuint	 id;
};

struct GLprogram {
	struct	GLshader **ss;
	int	nss;
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

struct	GLvarray *create_GLvarray(GLsizei, GLuint);
void	free_GLvarray(struct GLvarray *);

struct	GLbuffer *create_GLbuffer(GLsizei, GLuint);
void	free_GLbuffer(struct GLbuffer *);

struct	GLshader *create_GLshader(const char *, GLenum type);
void	free_GLshader(struct GLshader *);
GLuint	load_GLshader(struct GLshader *);

struct	GLprogram *create_GLprogram();
void	free_GLprogram(struct GLprogram *);
void	addshader_GLprogram(struct GLprogram *, struct GLshader *);
void	link_GLprogram(struct GLprogram *);

struct	GLunibuf *create_GLunibuf(struct GLprogram *, const char *);
void	free_GLunibuf(struct GLunibuf *);

void	print_program_log(GLuint);
void	print_shader_log(GLuint);
size_t	gl_sizeof(GLenum);
