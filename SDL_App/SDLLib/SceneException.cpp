/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#include "SceneException.h"
#include "Common.h"

namespace Library
{
	SceneException::SceneException(const char* const& func_name, const char* const& message)
		: runtime_error(message)
	{
		char buf[5000];
		SPRINTF(buf, "SceneException:%s : %s\n", func_name, message);
		mError = buf;
	}
	SceneException::SceneException(const char* const& message)
		: runtime_error(message)
	{
		char buf[5000];
		SPRINTF(buf, "SceneException: %s\n", message);
		mError = buf;
	}

	std::string SceneException::GetError() const
	{
		return mError;
	}
}