/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#pragma once

#ifdef _WIN32
#include <windows.h>
#endif
#include <exception>
#include <cassert>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "ServiceContainer.h"

#include "GL/glew.h"
#define GLM_FORCE_RADIANS
#ifdef __EMSCRIPTEN__
	#define GLM_FORCE_PURE 
#else
	#define GLM_FORCE_SSE2 
#endif
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/trigonometric.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/transform.hpp"

#include "RTTI.h"

#ifdef _MSC_VER
#define SPRINTF sprintf_s
#else
#define SPRINTF sprintf
#endif

namespace Library
{
#ifndef _WIN32
	typedef unsigned char byte;
#endif

	extern ServiceContainer GlobalServices;
}