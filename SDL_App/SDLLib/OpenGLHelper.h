/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#pragma once
#include "Common.h"

struct gl
{
	static void BindBuffers(GLuint vbo_id, GLuint index_id)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_id);
	}
	static void UnbindBuffers()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	static void AttribPtr(GLint id, GLint size, GLsizei stride, int offset)
	{
		glVertexAttribPointer(id, size, GL_FLOAT,
			GL_FALSE, stride * sizeof(GLfloat), (const void*)(offset * sizeof(GLfloat)));

		glEnableVertexAttribArray(id);
	}
	static void AttribIntPtr(GLint id, GLint size, GLsizei stride, int offset)
	{
		glVertexAttribPointer(id, size, GL_INT,
			GL_FALSE, stride * sizeof(GLfloat), (const void*)(offset * sizeof(GLfloat)));

		glEnableVertexAttribArray(id);
	}
	static void Send(GLint id, const glm::mat4& val)
	{
		glUniformMatrix4fv(id, 1, GL_FALSE, &val[0][0]);
	}
	static void Send(GLint id, const glm::vec4& val)
	{
		glUniform4fv(id, 1, glm::value_ptr(val));
	}
	static void Send(GLint id, const glm::vec3& val)
	{
		glUniform3fv(id, 1, glm::value_ptr(val));
	}
	static void Send(GLint id, GLfloat val)
	{
		glUniform1f(id, val);
	}
	static void Send(GLint id, GLint val)
	{
		glUniform1i(id, val);
	}
	static void Send(GLint id, GLuint val)
	{
		glUniform1ui(id, val);
	}
};

class FeatureRAII
{
public:
	FeatureRAII(GLenum feature) : mFeature(feature) { glEnable(mFeature); }
	~FeatureRAII() { glDisable(mFeature); }
private:
	GLenum mFeature;
};
