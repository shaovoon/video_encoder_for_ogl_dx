/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#include "ShaderProgram.h"
#include "SceneException.h"
#include "Utility.h"
#include "Model.h"
#include "Mesh.h"

namespace Library
{
	RTTI_DEFINITIONS(ShaderProgram)

	ShaderProgram::ShaderProgram()
		: mProgram(0), mVariables(), mVariablesByName(), mInit(false)
	{
		mProgram = glCreateProgram();
	}

	ShaderProgram::~ShaderProgram()
	{
		glDeleteProgram(mProgram);

		for (Variable* variable : mVariables)
		{
			delete variable;
		}
	}

	GLuint ShaderProgram::CompileShaderFromFile(GLenum shaderType, const std::wstring& filename, Precision precision)
	{
		// convert to ascii
		std::string filenameA = "";
		for (size_t i = 0; i < filename.size(); ++i)
		{
			filenameA += (char) filename[i];
		}
		std::string shaderSource;
		Utility::LoadShaderFile(filenameA, shaderSource, precision);
		const GLchar* sourcePointer = &shaderSource[0];
		GLint length = shaderSource.size();

		GLuint shader = glCreateShader(shaderType);
		glShaderSource(shader, 1, &sourcePointer, &length);
		glCompileShader(shader);

		GLint compileStatus;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus != GL_TRUE)
		{
			GLint logLength;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

			std::string log;
			log.reserve(logLength);

			glGetShaderInfoLog(shader, logLength, nullptr, const_cast<GLchar*>(log.c_str()));

			char buf[4000];
			SPRINTF(buf, "glCompileShader() failed: %s\n", log.c_str());

			throw SceneException(MY_FUNC, buf);
		}

		return shader;
	}

	GLuint ShaderProgram::CompileShaderFromSource(GLenum shaderType, const std::string& shaderSource, Precision precision)
	{
		std::string shader_src = "";
#ifndef _WIN32
		// insert the precision in the first line of the shader
		if (precision == Precision::highp)
			shader_src += "precision highp float;\n";
		else if (precision == Precision::mediump)
			shader_src += "precision mediump float;\n";
		else
			shader_src += "precision lowp float;\n";
#endif
		shader_src += shaderSource;

		const GLchar* sourcePointer = const_cast<GLchar*>(shader_src.data());
		GLint length = shader_src.size();

		GLuint shader = glCreateShader(shaderType);
		glShaderSource(shader, 1, &sourcePointer, &length);
		glCompileShader(shader);

		GLint compileStatus;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
		if (compileStatus != GL_TRUE)
		{
			GLint logLength;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

			std::string log;
			log.reserve(logLength);

			glGetShaderInfoLog(shader, logLength, nullptr, const_cast<GLchar*>(log.c_str()));

			char buf[4000];
			SPRINTF(buf, "glCompileShader() failed: %s\n", log.c_str());

			throw SceneException(MY_FUNC, buf);
		}

		return shader;
	}

	GLuint ShaderProgram::Program() const
	{
		return mProgram;
	}

	const std::vector<Variable*>& ShaderProgram::Variables() const
	{
		return mVariables;
	}

	const std::map<std::string, Variable*>& ShaderProgram::VariablesByName() const
	{
		return mVariablesByName;
	}

	void ShaderProgram::BuildProgram(const std::vector<ShaderDefinition>& shaderDefinitions, Precision precision)
	{
		std::vector<GLuint> compiledShaders;
		compiledShaders.reserve(shaderDefinitions.size());

		for (ShaderDefinition shaderDefiniton : shaderDefinitions)
		{
			GLuint compiledShader = CompileShaderFromFile(shaderDefiniton.first, shaderDefiniton.second, precision);
			glAttachShader(mProgram, compiledShader);
			compiledShaders.push_back(compiledShader);
		}

		glLinkProgram(mProgram);
		GLint linkStatus;
		glGetProgramiv(mProgram, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE)
		{
			GLint logLength;
			glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &logLength);

			std::string log;
			log.reserve(logLength);

			glGetProgramInfoLog(mProgram, logLength, nullptr, const_cast<GLchar*>(log.c_str()));

			char buf[4000];
			SPRINTF(buf, "glCompileShader() failed: %s\n", log.c_str());

			throw SceneException(MY_FUNC, buf);
		}

		for (GLuint compiledShader : compiledShaders)
		{
			glDeleteShader(compiledShader);
		}
		mInit = true;
	}

	void ShaderProgram::BuildProgramFromSource(const std::vector<ShaderSourceDefinition>& shaderDefinitions, Precision precision)
	{
		std::vector<GLuint> compiledShaders;
		compiledShaders.reserve(shaderDefinitions.size());

		for (ShaderSourceDefinition shaderDefiniton : shaderDefinitions)
		{
			GLuint compiledShader = CompileShaderFromSource(shaderDefiniton.first, shaderDefiniton.second, precision);
			glAttachShader(mProgram, compiledShader);
			compiledShaders.push_back(compiledShader);
		}

		glLinkProgram(mProgram);
		GLint linkStatus;
		glGetProgramiv(mProgram, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE)
		{
			GLint logLength;
			glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &logLength);

			std::string log;
			log.reserve(logLength);

			glGetProgramInfoLog(mProgram, logLength, nullptr, const_cast<GLchar*>(log.c_str()));

			char buf[4000];
			SPRINTF(buf, "glCompileShader() failed: %s\n", log.c_str());

			throw SceneException(MY_FUNC, buf);
		}

		for (GLuint compiledShader : compiledShaders)
		{
			glDeleteShader(compiledShader);
		}
		mInit = true;
	}

	void ShaderProgram::Use() const
	{
		glUseProgram(mProgram);
	}

	void ShaderProgram::CreateVertexBuffer(const Model& model, std::vector<GLuint>& vertexBuffers) const
	{
		vertexBuffers.reserve(model.Meshes().size());
		for (Mesh* mesh : model.Meshes())
		{
			GLuint vertexBuffer;
			CreateVertexBuffer(*mesh, vertexBuffer);
			vertexBuffers.push_back(vertexBuffer);
		}
	}

	void ShaderProgram::Initialize(GLuint vertexArrayObject)
	{
		glBindVertexArray(vertexArrayObject);
	}

	void ShaderProgram::CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer) const
	{
		throw SceneException(MY_FUNC, "ShaderProgram::CreateVertexBuffer() not implemented for base class.");
	}

	unsigned int ShaderProgram::VertexSize() const
	{
		throw SceneException(MY_FUNC, "ShaderProgram::VertexSize() not implemented for base class.");
	}

	GLint ShaderProgram::GetUniLoc(const char* var)
	{
		GLint loc = glGetUniformLocation(mProgram, var);
		if (loc == -1)
		{
			std::string err = "glGetUniformLocation() did not find ";
			err += var;
			throw SceneException(err.c_str());
		}
		return loc;
	}
	GLint ShaderProgram::GetAttLoc(const char* var)
	{
		GLint loc = glGetAttribLocation(mProgram, var);
		if (loc == -1)
		{
			std::string err = "glGetAttribLocation() did not find ";
			err += var;
			throw SceneException(err.c_str());
		}
		return loc;
	}

}