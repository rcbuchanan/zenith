#define ABS(x) (((x) > 0) ? (x) : (-(x)))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#ifndef M_PI
#define M_PI 3.1415925
#endif


#define MAX_SHADERS	10


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
	struct GLbuffer buf;
	GLuint id;
};

struct GLshader {
	GLuint type;
	GLchar *path;
	GLuint id;
};

struct GLprogram {
	struct GLshader *ss[MAX_SHADERS];
	int ns;
	GLuint id;
};

struct GLtexture {
	GLuint w;
	GLuint h;
	GLuint id;
};

/*
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
*/

void create_GLvarray(struct GLvarray *, GLsizei, GLuint);
void free_GLvarray(struct GLvarray *);

void create_GLbuffer(struct GLbuffer *, GLsizei, GLuint);
void bindonce_GLbuffer(struct GLbuffer *, GLenum, const void *);
void free_GLbuffer(struct GLbuffer *);

void create_GLframebuffer(struct GLframebuffer *, GLuint, GLuint, GLuint);
void rendertocube_GLframebuffer(struct GLframebuffer *, struct GLtexture *,
				GLenum);
void free_GLframebuffer(struct GLframebuffer *);

void create_GLshader(struct GLshader *, const char *, GLenum);
void free_GLshader(struct GLshader *);
GLuint load_GLshader(struct GLshader *);

void create_GLprogram(struct GLprogram *);
void free_GLprogram(struct GLprogram *);
void addshader_GLprogram(struct GLprogram *, struct GLshader *);
void addnewshader_GLprogram(struct GLprogram *, const char *, GLenum);
void link_GLprogram(struct GLprogram *);

void create_GLtexture(struct GLtexture *, GLuint, GLuint);
void loadtgacube_GLtexture(struct GLtexture *, char *[6]);
void framebuffercube_GLtexture(struct GLtexture *);
void free_GLtexture(struct GLtexture *);

void print_program_log(GLuint);
void print_shader_log(GLuint);
size_t gl_sizeof(GLenum);
