/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#include "Variable.h"
#include "SceneException.h"
#include "ShaderProgram.h"
#include "Utility.h"

namespace Library
{
	Variable::Variable(ShaderProgram& shaderProgram, const std::string& name)
		: mShaderProgram(shaderProgram), mLocation(-1), mName(name)
	{
		mLocation = mShaderProgram.GetUniLoc(name.c_str());
		if (mLocation == -1)
		{
			throw SceneException(MY_FUNC, "glGetUniformLocation() did not find uniform location.");
		}
	}

	ShaderProgram& Variable::GetShaderProgram()
	{
		return mShaderProgram;
	}

	const GLint& Variable::Location() const
	{
		return mLocation;
	}
	
	const std::string& Variable::Name() const
	{
		return mName;
	}

	Variable& Variable::operator<<(const glm::mat4& value)
	{
		glUniformMatrix4fv(mLocation, 1, GL_FALSE, &value[0][0]);

		return *this;
	}

	Variable& Variable::operator<<(const glm::vec4& value)
	{
		glUniform4fv(mLocation, 1, &value[0]);
	
		return *this;
	}

	Variable& Variable::operator<<(const glm::vec3& value)
	{
		glUniform3fv(mLocation, 1, &value[0]);

		return *this;
	}

	Variable& Variable::operator<<(const glm::vec2& value)
	{
		glUniform2fv(mLocation, 1, &value[0]);

		return *this;
	}

	Variable& Variable::operator<<(float value)
	{
		glUniform1f(mLocation, value);				

		return *this;
	}

	Variable& Variable::operator<<(int value)
	{
		glUniform1i(mLocation, value);

		return *this;
	}
}