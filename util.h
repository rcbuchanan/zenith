#define ABS(x) (((x) > 0) ? (x) : (-(x)))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


struct GLbuffer {
	GLuint n;
	GLsizei size;
	GLuint id;
};

struct GLframebuffer {
	GLuint id;
	GLuint w;
	GLuint h;
	GLuint hasrb;
	GLuint rbid;
};

struct GLvarray {
	struct GLbuffer *buf;
	GLuint id;
};

struct GLshader {
	GLuint type;
	GLchar *path;
	GLuint id;
};

struct GLprogram {
	struct GLshader **ss;
	int ns;
	GLuint id;
};

struct GLtexture {
	GLuint w;
	GLuint h;
	GLuint id;
};

struct GLunibuf {
	struct GLuni *unis;
	struct GLbuffer *buf;
	GLuint nuni;
	GLuint id;
};

struct GLuni {
	GLuint off;
	GLenum type;
	GLenum size;
	GLuint id;
};

struct GLvarray *create_GLvarray(GLsizei, GLuint);
void free_GLvarray(struct GLvarray *);

struct GLbuffer *create_GLbuffer(GLsizei, GLuint);
void bindonce_GLbuffer(struct GLbuffer *, GLenum, void *);
void free_GLbuffer(struct GLbuffer *);

struct GLframebuffer *create_GLframebuffer(GLuint, GLuint, GLuint);
void rendertocube_GLframebuffer(struct GLtexture *, GLenum);
void free_GLframebuffer(struct GLframebuffer *);

struct GLshader *create_GLshader(const char *, GLenum);
void free_GLshader(struct GLshader *);
GLuint load_GLshader(struct GLshader *);

struct GLprogram *create_GLprogram();
void free_GLprogram(struct GLprogram *);
void addshader_GLprogram(struct GLprogram *, struct GLshader *);
void link_GLprogram(struct GLprogram *);

struct GLtexture *create_GLtexture(GLuint, GLuint);
void loadtgacube_GLtexture(struct GLtexture *, char *[6]);
void framebuffercube_GLtexture(struct GLtexture *);
void free_GLtexture(struct GLtexture *);

struct GLunibuf *create_GLunibuf(struct GLprogram *, const char *);
void free_GLunibuf(struct GLunibuf *);

void print_program_log(GLuint);
void print_shader_log(GLuint);
size_t gl_sizeof(GLenum);
