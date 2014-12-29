#include <err.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "tga.h"
//#include <png.h>

#include "util.h"


#define MAX_SHADERS	10


static GLenum cube_axes[] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};

struct GLvarray *create_GLvarray(GLsizei s, GLuint n)
{
	struct GLvarray *v;

	if ((v = malloc(sizeof(struct GLvarray))) == NULL)
		errx(1, __FILE__ ": malloc");

	v->buf = create_GLbuffer(s, n);
	glGenVertexArrays(1, &(v->id));

	return v;
}

void free_GLvarray(struct GLvarray *v)
{
	errx(1, "free_GLvarray does not free in openGL!");

	if (v == NULL)
		return;
	if (v->buf != NULL)
		free_GLbuffer(v->buf);
}

struct GLbuffer *create_GLbuffer(GLsizei s, GLuint n)
{
	struct GLbuffer *b;

	if ((b = malloc(sizeof(struct GLbuffer))) == NULL)
		errx(1, __FILE__ ": malloc");

	b->n = n;
	b->size = s * n;

	glGenBuffers(1, &(b->id));

	return b;
}

void bindonce_GLbuffer(struct GLbuffer *b, GLenum type, void *v)
{
	glBindBuffer(type, b->id);
	glBufferData(type, b->size, v, GL_STATIC_DRAW);
}

void free_GLbuffer(struct GLbuffer *b)
{
	errx(1, "free_GLbuffer does not free in openGL!");

	if (b == NULL)
		return;
}

struct GLframebuffer *create_GLframebuffer(GLuint w, GLuint h, GLuint usedepth)
{
	struct GLframebuffer *f;

	if ((f = malloc(sizeof(struct GLframebuffer))) == NULL)
		errx(1, __FILE__ ": malloc");

	f->w = w;
	f->h = h;
	f->hasrb = 0;
	f->rbid = 0;

	glGenFramebuffers(1, &f->id);
	glBindFramebuffer(GL_FRAMEBUFFER, f->id);

	if (!usedepth)
		return NULL;

	f->hasrb = 1;

	glGenRenderbuffers(1, &f->rbid);
	glBindRenderbuffer(GL_RENDERBUFFER, f->rbid);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, f->w, f->h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				  GL_RENDERBUFFER, f->rbid);

	return f;
}

void rendertocube_GLframebuffer(struct GLframebuffer *f, struct GLtexture *t,
				GLenum face)
{
	GLenum tmp;

	glBindFramebuffer(GL_FRAMEBUFFER, f->id);
	glViewport(0, 0, t->w, t->h);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, face,
			       t->id, 0);

	tmp = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &tmp);
}

void free_GLframebuffer(struct GLframebuffer *f)
{
	errx(1, "free_GLframebuffer does not free in OpenGL");
}

struct GLprogram *create_GLprogram()
{
	struct GLprogram *p;

	if ((p = malloc(sizeof(struct GLprogram))) == NULL)
		errx(1, __FILE__ ": malloc");

	if ((p->ss = malloc(sizeof(struct GLshader *) * MAX_SHADERS)) == NULL)
		errx(1, __FILE__ ": malloc");

	p->ns = 0;
	p->id = glCreateProgram();

	return p;
}

void addshader_GLprogram(struct GLprogram *p, struct GLshader *s)
{
	if (p->ns >= MAX_SHADERS)
		errx(1, __FILE__ ": too many shaders attached!");
	p->ss[p->ns++] = s;
}


void link_GLprogram(struct GLprogram *p)
{
	int i;

	for (i = 0; i < p->ns; i++)
		glAttachShader(p->id, p->ss[i]->id);
	glLinkProgram(p->id);
	for (i = 0; i < p->ns; i++)
		glDetachShader(p->id, p->ss[i]->id);

	print_program_log(p->id);
}

void free_GLprogram(struct GLprogram *p)
{
	glDeleteProgram(p->id);
}

struct GLshader *create_GLshader(const char *sfile, GLenum type)
{
	struct GLshader *s;

	if ((s = malloc(sizeof(struct GLshader))) == NULL)
		errx(1, __FILE__ ": malloc");

	s->type = type;
	if ((s->path = malloc(strlen(sfile) + 1)) == NULL)
		errx(1, __FILE__ ": malloc");
	strcpy(s->path, sfile);

	s->id = glCreateShader(type);
	if (s->id == 0 || s->id == GL_INVALID_ENUM)
		errx(1, __FILE__ ": createShader");

	if (load_GLshader(s))
		errx(1, __FILE__ ": failed to compile shader");

	return s;
}

GLuint load_GLshader(struct GLshader * s)
{
	GLchar *buf;
	FILE *f;
	size_t len;
	GLint rv;

	if (s == NULL || s->path == NULL || s->id == 0)
		goto failure;

	if ((f = fopen(s->path, "r")) == NULL)
		goto failure;

	if (fseek(f, 0, SEEK_END))
		goto failure;
	len = ftell(f);
	if (fseek(f, 0, SEEK_SET))
		goto failure;

	if ((buf = malloc(len * sizeof(GLchar))) == NULL)
		goto failure;
	if (fread(buf, 1, (size_t) len, f) != len)
		goto failure;

	glShaderSource(s->id, 1, (const GLchar **) &buf, (const GLint *) &len);
	glCompileShader(s->id);
	print_shader_log(s->id);

	rv = 0;

	goto cleanup;

failure:
	warnx(__FILE__ ": trouble loading %s", s->path);
	rv = -1;

cleanup:
	if (buf != NULL)
		free(buf);
	if (f != NULL)
		fclose(f);

	return rv;
}

void free_GLshader(struct GLshader *s)
{
	if (s->path)
		free(s->path);
	glDeleteShader(s->id);
}

struct GLtexture *create_GLtexture(GLuint w, GLuint h)
{
	struct GLtexture *t;

	if ((t = malloc(sizeof(struct GLtexture))) == NULL)
		errx(1, __FILE__ ": malloc");

	t->w = w;
	t->h = h;
	glGenTextures(1, &t->id);

	return t;
}

void loadtgacube_GLtexture(struct GLtexture *t, char *fpaths[6])
{
	tTGA ttga[6];
	GLenum mode;
	int i;

	glBindTexture(GL_TEXTURE_CUBE_MAP, t->id);

	for (i = 0; i < ARRAY_SIZE(cube_axes); i++) {
		load_TGA(ttga + i, fpaths[i]);
		mode = ttga[i].alpha ? GL_RGBA : GL_RGB;
		glTexImage2D(cube_axes[i], 0, mode, t->w, t->h,
			     0, mode, GL_UNSIGNED_BYTE, ttga[i].data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	for (i = 0; i < 6; i++)
		free_TGA(ttga + i);
}

void framebuffercube_GLtexture(struct GLtexture *t)
{
	int i;

	glBindTexture(GL_TEXTURE_CUBE_MAP, t->id);

	for (i = 0; i < ARRAY_SIZE(cube_axes); i++) {
		glTexImage2D(cube_axes[i], 0, GL_RGB, t->w, t->h,
			     0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

/*
void png_GLtexture(struct GLtexture *t, char *fn)
{
	png_structp pngp;
	png_infop infop;
	png_bytep *rp;
	GLuint w, h;
	GLuint i;
	GLint depth, colortype, npasses;
	FILE *f;
	unsigned char sig[8];

	// all I want to do is load a stupid png...
	// based on http://zarb.org/~gc/html/libpng.html

	f = fopen(fn, "rb");
	if (f == NULL)
		errx(1, __FILE__ ": couldn't open %s", fn);
	if (fread(sig, 1, 8, f) != 8 || png_sig_cmp(sig, 0, 8))
		errx(1, __FILE__ ": confusing png sig for %s", fn);

	pngp = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (pngp == NULL)
		errx(1, __FILE__ ": png trouble");

	infop = png_create_info_struct(pngp);
	if (infop == NULL)
		errx(1, __FILE__ ": png");

	if (setjmp(png_jmpbuf(pngp)))
		errx(1, __FILE__ ": png longjumped error");

	png_init_io(pngp, f);
	png_set_sig_bytes(pngp, 8);

	png_read_info(pngp, infop);

	w = png_get_image_width(pngp, infop);
	h = png_get_image_height(pngp, infop);
	colortype = png_get_color_type(pngp, infop);
	depth = png_get_bit_depth(pngp, infop);
	npasses = png_set_interlace_handling(pngp);

	png_read_update_info(pngp, infop);

	if (setjmp(png_jmpbuf(pngp)))
		errx(1, __FILE__ ": png read_image error");

	rp = (png_bytep *) malloc(sizeof(png_bytep) * h);

	for (i = 0; i < h; i++)
		rp[i] = malloc(png_get_rowbytes(pngp, infop));

	png_destroy_read_struct(&pngp, &infop, png_infopp_NULL);

	fclose(f);
}
*/

struct GLunibuf *create_GLunibuf(struct GLprogram *p, const char *name)
{
	struct GLunibuf *u = NULL;
	GLsizei bs;
	GLuint *is;
	int i;

	if ((u = malloc(sizeof(struct GLunibuf))) == NULL)
		errx(1, __FILE__ ": malloc");

	u->id = glGetUniformBlockIndex(p->id, name);
	if (u->id == GL_INVALID_INDEX)
		goto failed;

	glGetActiveUniformBlockiv(p->id,
				  u->id, GL_UNIFORM_BLOCK_DATA_SIZE, &bs);
	if (bs == 0)
		goto failed;

	u->buf = create_GLbuffer(bs, 1);

	glGetActiveUniformBlockiv(p->id,
				  u->id,
				  GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS,
				  (GLint *) & u->nuni);

	if (u->nuni != 0) {
		if ((u->unis = malloc(sizeof(struct GLuni) * u->nuni)) == NULL)
			goto failed;

		if ((is = malloc(sizeof(GLint) * u->nuni)) == NULL)
			goto failed;

		glGetActiveUniformBlockiv(p->id,
					  u->id,
					  GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES,
					  (GLint *) is);

		for (i = 0; i < u->nuni; i++) {
			u->unis[i].id = is[i];
			glGetActiveUniformsiv(p->id,
					      1,
					      (const GLuint *) is + i,
					      GL_UNIFORM_OFFSET,
					      (GLint *) & u->unis[i].off);
			glGetActiveUniformsiv(p->id,
					      1,
					      (const GLuint *) is + i,
					      GL_UNIFORM_SIZE,
					      (GLint *) & u->unis[i].size);
			glGetActiveUniformsiv(p->id,
					      1,
					      (const GLuint *) is + i,
					      GL_UNIFORM_TYPE,
					      (GLint *) & u->unis[i].type);
		}

		free(is);
	}

	return u;

      failed:
	errx(1, __FILE__ ": couldn't create unibuf for %s", name);
	free_GLprogram(p);

	return NULL;
}

void free_GLunibuf(struct GLunibuf *u)
{
	free_GLbuffer(u->buf);
	free(u->unis);
}

void print_program_log(GLuint prog)
{
	GLsizei len = 0;
	GLchar *buf;

	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if ((buf = malloc(sizeof(GLchar) * len + 1)) == NULL)
		errx(1, __FILE__ ": malloc");
	buf[len] = '\0';

	glGetProgramInfoLog(prog, len, &len, buf);
	printf("%s\n", (char *) buf);

	free(buf);
}

void print_shader_log(GLuint shader)
{
	GLsizei len = 0;
	GLchar *buf;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if ((buf = malloc(sizeof(GLchar) * len + 1)) == NULL)
		errx(1, __FILE__ ": malloc");
	buf[len] = '\0';

	glGetShaderInfoLog(shader, len, &len, buf);
	printf("%s\n", (char *) buf);

/*cleanup:*/
	free(buf);
}

size_t gl_sizeof(GLenum type)
{
	size_t size = 0;

#define CASE(Enum, Count, Type) \
		case Enum: size = Count * sizeof(Type); break
	switch (type) {
		  CASE(GL_FLOAT, 1, GLfloat);
		  CASE(GL_FLOAT_VEC2, 2, GLfloat);
		  CASE(GL_FLOAT_VEC3, 3, GLfloat);
		  CASE(GL_FLOAT_VEC4, 4, GLfloat);
		  CASE(GL_INT, 1, GLint);
		  CASE(GL_INT_VEC2, 2, GLint);
		  CASE(GL_INT_VEC3, 3, GLint);
		  CASE(GL_INT_VEC4, 4, GLint);
		  CASE(GL_UNSIGNED_INT, 1, GLuint);
		  CASE(GL_UNSIGNED_INT_VEC2, 2, GLuint);
		  CASE(GL_UNSIGNED_INT_VEC3, 3, GLuint);
		  CASE(GL_UNSIGNED_INT_VEC4, 4, GLuint);
		  CASE(GL_BOOL, 1, GLboolean);
		  CASE(GL_BOOL_VEC2, 2, GLboolean);
		  CASE(GL_BOOL_VEC3, 3, GLboolean);
		  CASE(GL_BOOL_VEC4, 4, GLboolean);
		  CASE(GL_FLOAT_MAT2, 6, GLfloat);
		  CASE(GL_FLOAT_MAT2x4, 8, GLfloat);
		  CASE(GL_FLOAT_MAT3, 9, GLfloat);
		  CASE(GL_FLOAT_MAT3x2, 6, GLfloat);
		  CASE(GL_FLOAT_MAT3x4, 12, GLfloat);
		  CASE(GL_FLOAT_MAT4, 16, GLfloat);
		  CASE(GL_FLOAT_MAT4x2, 8, GLfloat);
		  CASE(GL_FLOAT_MAT4x3, 12, GLfloat);
#undef CASE
	  default:
		  errx(1, __FILE__ ": Unknown type: 0x%x", type);
		  break;
	}

	return size;
}
