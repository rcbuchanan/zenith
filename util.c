#include <err.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "util.h"


struct GLbuffer *
create_GLbuffer(GLsizei s, GLuint n)
{
	struct	GLbuffer *b;

	if ((b = malloc(sizeof (struct GLbuffer))) == NULL)
		errx(1, "%s: malloc", __FILE__);

	b->n = n;
	b->size = s * n;
	if ((b->d = malloc(b->size)) == NULL)
		errx(1, "%s: malloc", __FILE__);

	glGenBuffers(1, &(b->id));

	return b;
}

void
free_GLbuffer(struct GLbuffer *b)
{
	free(b->d);
	free(b);
}

struct GLprogram *
create_GLprogram(const char *vsfile, const char *fsfile)
{
	struct	GLprogram *p;

	if ((p = malloc(sizeof (struct GLprogram))) == NULL)
		errx(1, "%s: malloc", __FILE__);

	p->id = glCreateProgram();
	p->vsid = load_shader(vsfile, GL_VERTEX_SHADER);
	p->fsid = load_shader(fsfile, GL_FRAGMENT_SHADER);

	if (p->id == 0 || p->vsid == 0 || p->fsid == 0)
		goto failed;

	glAttachShader(p->id, p->vsid);
	glAttachShader(p->id, p->fsid);
	glLinkProgram(p->id);
	print_program_log(p->id);

	return p;

failed:
	warnx("%s: couldn't create program", __FILE__);
	free_GLprogram(p);

	return NULL;
}

void
free_GLprogram(struct GLprogram *p)
{
	free(p);
}

struct GLunibuf *
create_GLunibuf(struct GLprogram *p, const char *name)
{
	struct	 GLunibuf *u = NULL;
	GLsizei	 bs;
	GLuint	*is;
	int	 i;

	if ((u = malloc(sizeof (struct GLunibuf))) == NULL)
		errx(1, "%s: malloc", __FILE__);

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
	warnx("%s: couldn't create unibuf for %s", __FILE__, name);
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
		errx(1, "%s: malloc", __FILE__);
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
		errx(1, "%s: malloc", __FILE__);
	buf[len] = '\0';

	glGetShaderInfoLog(shader, len, &len, buf);
	printf("%s\n", (char *) buf);

/*cleanup:*/
	free(buf);
}

GLuint
load_shader(const char *filename, GLenum type)
{
	GLchar	*buf;
	FILE	*f;
	size_t	 len;
	GLuint	 rv = 0;

	if ((f = fopen(filename, "r")) == NULL)
		goto failed;

	if (fseek(f, 0, SEEK_END))
		goto failed;
	len = ftell(f) * sizeof (char);
	if (fseek(f, 0, SEEK_SET))
		goto failed;

	if ((buf = malloc(len * sizeof (GLchar))) == NULL)
		errx(1, "%s: malloc", __FILE__);
	if (fread(buf, sizeof (char), (size_t) len, f) != len)
		goto failed;

	rv = glCreateShader(type);
	glShaderSource(rv, 1, (const GLchar **) &buf, (const GLint *) &len);
	glCompileShader(rv);
	print_shader_log(rv);

	goto cleanup;

failed:
	warnx("%s: failed to load shader %s", __FILE__, filename);

cleanup:
	free(buf);

	return rv;
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
	errx(1, "%s: Unknown type: 0x%x\n", __FILE__, type);
	break;
	}

	return size;
}
