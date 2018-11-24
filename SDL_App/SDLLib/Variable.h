/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#pragma once

#include "Common.h"

namespace Library
{
	class ShaderProgram;

    class Variable
    {
    public:
		Variable(ShaderProgram& shaderProgram, const std::string& name);		

        ShaderProgram& GetShaderProgram();
		const GLint& Location() const;
        const std::string& Name() const;

        Variable& operator<<(const glm::mat4& value);
        Variable& operator<<(const glm::vec4& value);
		Variable& operator<<(const glm::vec3& value);
		Variable& operator<<(const glm::vec2& value);        
		Variable& operator<<(float value);
		Variable& operator<<(int value);

    private:
        Variable(const Variable& rhs);
        Variable& operator=(const Variable& rhs);

		ShaderProgram& mShaderProgram;
		GLint mLocation;
        std::string mName;
    };
}