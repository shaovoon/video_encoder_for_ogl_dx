/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#pragma once

#include "Common.h"

namespace Library
{
	class Vector2Helper
	{
	public:
		static const glm::vec2 Zero;
		static const glm::vec2 One;

		static std::string ToString(const glm::vec2& vector);

	private:
		Vector2Helper();
		Vector2Helper(const Vector2Helper& rhs);
		Vector2Helper& operator=(const Vector2Helper& rhs);
	};

	class Vector3Helper
	{
	public:
		static const glm::vec3 Zero;
		static const glm::vec3 One;
		static const glm::vec3 Forward;
		static const glm::vec3 Backward;
		static const glm::vec3 Up;
		static const glm::vec3 Down;
		static const glm::vec3 Right;
		static const glm::vec3 Left;

		static std::string ToString(const glm::vec3& vector);

	private:
		Vector3Helper();
		Vector3Helper(const Vector3Helper& rhs);
		Vector3Helper& operator=(const Vector3Helper& rhs);
	};

	class Vector4Helper
	{
	public:
		static const glm::vec4 Zero;
		static const glm::vec4 One;

		static std::string ToString(const glm::vec4& vector);

	private:
		Vector4Helper();
		Vector4Helper(const Vector3Helper& rhs);
		Vector4Helper& operator=(const Vector3Helper& rhs);
	};
}