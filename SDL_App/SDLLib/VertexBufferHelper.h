/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#pragma once
#include "Common.h"

class VertexBufferHelper
{
public:
	VertexBufferHelper();
	~VertexBufferHelper();

	static void createVBO(GLfloat *vertexBuffer,
		GLushort *indices, GLuint numVertices, GLuint numIndices,
		GLuint& vboId, GLuint& indexId, GLuint elems_in_vertex, GLenum usage);

	static void createVBO(GLfloat *vertexBuffer,
		GLuint numVertices, GLuint& vboId, GLuint numElems, GLenum usage);

	static void updateVBO(GLfloat* vertices, size_t size);


};

