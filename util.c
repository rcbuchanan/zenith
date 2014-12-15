#include <err.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "util.h"


#define MAX_SHADERS	10


struct GLvarray *
create_GLvarray(GLsizei s, GLuint n)
{
	struct	GLvarray *v;

	if ((v = malloc (sizeof (struct GLvarray))) == NULL)
		errx(1, __FILE__ ": malloc");

	v->buf = create_GLbuffer(s, n);
	glGenVertexArrays(1, &(v->id));

	return v;
}

void
free_GLvarray(struct GLvarray *v)
{
	errx(1, "free_GLvarray does not free in openGL!");

	if (v == NULL)		return;
	if (v->buf != NULL)	free_GLbuffer(v->buf);
	free(v);
}

struct GLbuffer *
create_GLbuffer(GLsizei s, GLuint n)
{
	struct	GLbuffer *b;

	if ((b = malloc(sizeof (struct GLbuffer))) == NULL)
		errx(1, __FILE__ ": malloc");

	b->n = n;
	b->size = s * n;
	if ((b->d = malloc(b->size)) == NULL)
		errx(1, __FILE__ ": malloc");

	glGenBuffers(1, &(b->id));

	return b;
}

void
free_GLbuffer(struct GLbuffer *b)
{
	errx(1, "free_GLbuffer does not free in openGL!");

	if (b == NULL)		return;
	if (b->d != NULL)	free(b->d);
	free(b);
}

struct GLprogram *
create_GLprogram()
{
	struct	GLprogram *p;

	if ((p = malloc(sizeof (struct GLprogram))) == NULL)
		errx(1, __FILE__ ": malloc");

	if ((p->ss = malloc(sizeof (struct GLshader *) * MAX_SHADERS)) == NULL)
		errx(1, __FILE__ ": malloc");

	p->nss = 0;

	p->id = glCreateProgram();

	return p;
}

void
addshader_GLprogram(struct GLprogram *p, struct GLshader *s)
{
	if (p->nss >= MAX_SHADERS)
		errx(1, __FILE__ ": too many shaders attached!");
	p->ss[p->nss++] = s;
}


void
link_GLprogram(struct GLprogram *p)
{
	int	i;

	for (i = 0; i < p->nss; i++)	glAttachShader(p->id, p->ss[i]->id);
	glLinkProgram(p->id);
	for (i = 0; i < p->nss; i++)	glDetachShader(p->id, p->ss[i]->id);

	print_program_log(p->id);
}

void
free_GLprogram(struct GLprogram *p)
{
	glDeleteProgram(p->id);
	free(p);
}

struct GLshader *
create_GLshader(const char *sfile, GLenum type)
{
	struct	GLshader *s;

	if ((s = malloc(sizeof (struct GLshader))) == NULL)
		errx(1, __FILE__ ": malloc");

	s->type = type;
	if ((s->path = malloc(strlen (sfile) + 1)) == NULL)
		errx(1, __FILE__ ": malloc");
	strcpy(s->path, sfile);

	s->id = glCreateShader(type);
	if (s->id == 0 || s->id == GL_INVALID_ENUM)
		errx(1, __FILE__ ": createShader");

	if (load_GLshader(s))
		errx(1, __FILE__ ": failed to compile shader");

	return s;
}

GLuint
load_GLshader(struct GLshader *s)
{
	GLchar	*buf;
	FILE	*f;
	size_t	 len;
	GLint	 rv;

	if (s == NULL || s->path == NULL || s->id == 0)
		goto failure;

	if ((f = fopen(s->path, "r")) == NULL)
		goto failure;

	if (fseek(f, 0, SEEK_END))
		goto failure;
	len = ftell(f);
	if (fseek(f, 0, SEEK_SET))
		goto failure;

	if ((buf = malloc(len * sizeof (GLchar))) == NULL)
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
	if (buf != NULL)	free(buf);
	if (f != NULL)		fclose(f);

	return rv;
}

void
free_GLshader(struct GLshader *s)
{
	if (s->path)	free(s->path);
	glDeleteShader(s->id);
	free(s);
}

struct GLunibuf *
create_GLunibuf(struct GLprogram *p, const char *name)
{
	struct	 GLunibuf *u = NULL;
	GLsizei	 bs;
	GLuint	*is;
	int	 i;

	if ((u = malloc(sizeof (struct GLunibuf))) == NULL)
		errx(1, __FILE__ ": malloc");

	u->id = glGetUniformBlockIndex(p->id, name);
	if (u->id == GL_INVALID_INDEX)
		goto failed;

	glGetActiveUniformBlockiv(
		p->id,
		u->id,
		GL_UNIFORM_BLOCK_DATA_SIZE,
		&bs);
	if (bs == 0)
		goto failed;

	u->buf = create_GLbuffer(bs, 1);

	glGetActiveUniformBlockiv(
		p->id,
		u->id,
		GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS,
		(GLint *) &u->nuni);

	if (u->nuni != 0) {
		if ((u->unis = malloc(sizeof (struct GLuni) * u->nuni)) == NULL)
			goto failed;

		if ((is = malloc(sizeof (GLint) * u->nuni)) == NULL)
			goto failed;

		glGetActiveUniformBlockiv(
			p->id,
			u->id,
			GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES,
			(GLint *) is);

		for (i = 0; i < u->nuni; i++) {
			u->unis[i].id = is[i];
			glGetActiveUniformsiv(
				p->id,
				1,
				(const GLuint *) is + i,
				GL_UNIFORM_OFFSET,
				(GLint *) &u->unis[i].off);
			glGetActiveUniformsiv(
				p->id,
				1,
				(const GLuint *) is + i,
				GL_UNIFORM_SIZE,
				(GLint *) &u->unis[i].size);
			glGetActiveUniformsiv(
				p->id,
				1,
				(const GLuint *) is + i,
				GL_UNIFORM_TYPE,
				(GLint *) &u->unis[i].type);
		}

		free (is);
	}

	return u;

failed:
	errx(1, __FILE__ ": couldn't create unibuf for %s", name);
	free_GLprogram(p);

	return NULL;
}

void
free_GLunibuf(struct GLunibuf *u)
{
	free_GLbuffer(u->buf);
	free(u->unis);
	free(u);
}

void
print_program_log(GLuint prog)
{
	GLsizei	 len = 0;
	GLchar	*buf;

	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if ((buf = malloc(sizeof (GLchar) * len + 1 )) == NULL)
		errx(1, __FILE__ ": malloc");
	buf[len] = '\0';

	glGetProgramInfoLog(prog, len, &len, buf);
	printf("%s\n", (char *) buf);

/*cleanup:*/
	free(buf);
}

void
print_shader_log(GLuint shader)
{
	GLsizei	len = 0;
	GLchar	*buf;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if ((buf = malloc(sizeof (GLchar) * len + 1)) == NULL)
		errx(1, __FILE__ ": malloc");
	buf[len] = '\0';

	glGetShaderInfoLog(shader, len, &len, buf);
	printf("%s\n", (char *) buf);

/*cleanup:*/
	free(buf);
}

size_t
gl_sizeof(GLenum type)
{
	size_t	size = 0;

	#define CASE(Enum, Count, Type) \
		case Enum: size = Count * sizeof(Type); break
	switch (type) {
	CASE(GL_FLOAT,			1,	GLfloat);
	CASE(GL_FLOAT_VEC2,		2,	GLfloat);
	CASE(GL_FLOAT_VEC3,		3,	GLfloat);
	CASE(GL_FLOAT_VEC4,		4,	GLfloat);
	CASE(GL_INT,			1,	GLint);
	CASE(GL_INT_VEC2,		2,	GLint);
	CASE(GL_INT_VEC3,		3,	GLint);
	CASE(GL_INT_VEC4,		4,	GLint);
	CASE(GL_UNSIGNED_INT,		1,	GLuint);
	CASE(GL_UNSIGNED_INT_VEC2,	2,	GLuint);
	CASE(GL_UNSIGNED_INT_VEC3,	3,	GLuint);
	CASE(GL_UNSIGNED_INT_VEC4,	4,	GLuint);
	CASE(GL_BOOL,			1,	GLboolean);
	CASE(GL_BOOL_VEC2,		2,	GLboolean);
	CASE(GL_BOOL_VEC3,		3,	GLboolean);
	CASE(GL_BOOL_VEC4,		4,	GLboolean);
	CASE(GL_FLOAT_MAT2,		6,	GLfloat);
	CASE(GL_FLOAT_MAT2x4,		8,	GLfloat);
	CASE(GL_FLOAT_MAT3,		9,	GLfloat);
	CASE(GL_FLOAT_MAT3x2,		6,	GLfloat);
	CASE(GL_FLOAT_MAT3x4,		12,	GLfloat);
	CASE(GL_FLOAT_MAT4,		16,	GLfloat);
	CASE(GL_FLOAT_MAT4x2,		8,	GLfloat);
	CASE(GL_FLOAT_MAT4x3,		12,	GLfloat);
	#undef CASE
	default:
	errx(1, __FILE__ ": Unknown type: 0x%x", type);
	break;
	}

	return size;
}
