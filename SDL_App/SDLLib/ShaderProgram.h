/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#pragma once

#include "Common.h"
#include "Variable.h"
#include "Enums.h"
#include "SceneException.h"
#include "OpenGLHelper.h"

namespace Library
{
	class Model;
	class Mesh;

	typedef std::pair<GLenum, std::wstring> ShaderDefinition;
	typedef std::pair<GLenum, std::string> ShaderSourceDefinition;

	class ShaderProgram : public RTTI
	{
		RTTI_DECLARATIONS(ShaderProgram, RTTI)

	public:
		ShaderProgram();
		virtual ~ShaderProgram();
		
		static GLuint CompileShaderFromFile(GLenum shaderType, const std::wstring& filename, Precision precision);
		static GLuint CompileShaderFromSource(GLenum shaderType, const std::string& shaderSource, Precision precision);
		
		Variable* operator[](const std::string& variableName);

		GLuint Program() const;	
		const std::vector<Variable*>& Variables() const;
		const std::map<std::string, Variable*>& VariablesByName() const;

		void BuildProgram(const std::vector<ShaderDefinition>& shaderDefinitions, Precision precision = Precision::mediump);
		void BuildProgramFromSource(const std::vector<ShaderSourceDefinition>& shaderDefinitions, Precision precision = Precision::mediump);

		virtual void Initialize(GLuint vertexArrayObject);
		virtual void Use() const;
		virtual void CreateVertexBuffer(const Model& model, std::vector<GLuint>& vertexBuffers) const;
		virtual void CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer) const;
		virtual unsigned int VertexSize() const;
		
		GLint GetUniLoc(const char* var);
		GLint GetAttLoc(const char* var);

		bool IsNull() const { return mInit == false; }

	protected:
		GLuint mProgram;
		std::vector<Variable*> mVariables;
		std::map<std::string, Variable*> mVariablesByName;
		bool mInit;

	private:
		ShaderProgram(const ShaderProgram& rhs);
		ShaderProgram& operator=(const ShaderProgram& rhs);	
	};

	#define SHADER_VARIABLE_DECLARATION(VariableName)   \
		public:											\
		Variable& VariableName() const;					\
	private:											\
		Variable* m ## VariableName;

	#define SHADER_VARIABLE_DEFINITION(ShaderProgram, VariableName)	\
		Variable& ShaderProgram::VariableName() const				\
		{															\
			return *m ## VariableName;								\
		}

	#define SHADER_VARIABLE_INITIALIZATION(VariableName) m ## VariableName(NULL)

	#define SHADER_VARIABLE_INSTANTIATE(VariableName)															\
		m ## VariableName = new Variable(*this, #VariableName);													\
		mVariables.push_back(m ## VariableName);																\
		mVariablesByName.insert(std::pair<std::string, Variable*>(m ## VariableName->Name(), m ## VariableName));
}
