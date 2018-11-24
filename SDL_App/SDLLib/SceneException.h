/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#pragma once

#include <exception>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <string>

namespace Library
{
	class SceneException : public std::runtime_error
	{
	public:
		SceneException(const char* const& func_name, const char* const& message);
		SceneException(const char* const& message);

		std::string GetError() const;
	private:
		std::string mError;
	};
}