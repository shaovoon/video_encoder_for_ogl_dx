/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#include "VertexBufferHelper.h"


VertexBufferHelper::VertexBufferHelper()
{
}


VertexBufferHelper::~VertexBufferHelper()
{
}

void VertexBufferHelper::createVBO(GLfloat *vertexBuffer,
	GLushort *indices, GLuint numVertices, GLuint numIndices,
	GLuint& vboId, GLuint& indexId, GLuint numElems, GLenum usage)
{
	// vertex
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, numVertices*(sizeof(GLfloat) * numElems),
		vertexBuffer, usage);

	// bind buffer object for element indices
	glGenBuffers(1, &indexId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		numIndices * sizeof(GLushort), indices,
		usage);
}

void VertexBufferHelper::createVBO(GLfloat *vertexBuffer,
	GLuint numVertices, GLuint& vboId, GLuint numElems, GLenum usage)
{
	// vertex
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, numVertices*(sizeof(GLfloat) * numElems),
		vertexBuffer, usage);
}

void VertexBufferHelper::updateVBO(GLfloat* vertices, size_t size)
{
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, (const GLvoid *) vertices);
}

