#include <GL/glew.h>

const GLfloat cube_vtx[8 * 3] = {
	1.000000 , -1.000000, -1.000000,
	1.000000 , -1.000000,  1.000000,
	-1.000000, -1.000000,  1.000000,
	-1.000000, -1.000000, -1.000000,
	1.000000 ,  1.000000, -1.000000,
	1.000000 ,  1.000000,  1.000000,
	-1.000000,  1.000000,  1.000000,
	-1.000000,  1.000000, -1.000000
};

const GLuint cube_faces[12 * 3] = {
	0, 1, 2,
	4, 7, 6,
	0, 4, 5,
	1, 5, 6,
	2, 6, 7,
	4, 0, 3,

	0, 2, 3,
	4, 6, 5,
	0, 5, 1,
	1, 6, 2,
	2, 7, 3,
	4, 3, 7,
};

const GLfloat ico_vtx[12][3] = {
	{ 0.00000000,  0.00000000, -0.95105650},
	{ 0.00000000,  0.85065080, -0.42532537},
	{ 0.80901698,  0.26286556, -0.42532537},
	{ 0.50000000, -0.68819095, -0.42532537},
	{-0.50000000, -0.68819095, -0.42532537},
	{-0.80901698,  0.26286556, -0.42532537},
	{ 0.50000000,  0.68819095,  0.42532537},
	{ 0.80901698, -0.26286556,  0.42532537},
	{ 0.00000000, -0.85065080,  0.42532537},
	{-0.80901698, -0.26286556,  0.42532537},
	{-0.50000000,  0.68819095,  0.42532537},
	{ 0.00000000,  0.00000000,  0.95105650}
};

const GLuint ico_faces[20][3] = {
	{ 1,  2,  0},
	{ 2,  3,  0},
	{ 3,  4,  0},
	{ 4,  5,  0},
	{ 5,  1,  0},
	{10,  6,  1},
	{ 6,  2,  1},
	{ 6,  7,  2},
	{ 7,  3,  2},
	{ 7,  8,  3},
	{ 8,  4,  3},
	{ 8,  9,  4},
	{ 9,  5,  4},
	{ 9, 10,  5},
	{10,  1,  5},
	{11,  6, 10},
	{11,  7,  6},
	{11,  8,  7},
	{11,  9,  8},
	{11, 10,  9}
};
