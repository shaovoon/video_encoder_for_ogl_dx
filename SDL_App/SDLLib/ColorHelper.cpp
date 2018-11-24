/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "ColorHelper.h"

namespace Library
{
	std::random_device ColorHelper::sDevice;
	std::default_random_engine ColorHelper::sGenerator(sDevice());
	std::uniform_real_distribution<float> ColorHelper::sDistribution(0, 1);

	const glm::vec4 ColorHelper::NaturalBlack =     glm::vec4(0.08235f, 0.09803f, 0.14117f, 1.0f);
	const glm::vec4 ColorHelper::Black =			glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	const glm::vec4 ColorHelper::White =			glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	const glm::vec4 ColorHelper::Red =				glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	const glm::vec4 ColorHelper::Green =			glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	const glm::vec4 ColorHelper::Blue =				glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	const glm::vec4 ColorHelper::Yellow =			glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	const glm::vec4 ColorHelper::BlueGreen =		glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
	const glm::vec4 ColorHelper::Purple =			glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
	const glm::vec4 ColorHelper::CornflowerBlue =	glm::vec4(0.392f, 0.584f, 0.929f, 1.0f);
	const glm::vec4 ColorHelper::Wheat =			glm::vec4(0.961f, 0.871f, 0.702f, 1.0f);
	const glm::vec4 ColorHelper::LightGray =		glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

	glm::vec4 ColorHelper::RandomColor()
	{
		float r = sDistribution(sGenerator);
		float g = sDistribution(sGenerator);
		float b = sDistribution(sGenerator);

		return glm::vec4(r, g, b, 1.0f);
	}

	glm::vec4 ColorHelper::rgb2hsv(const glm::vec4& in)
	{
		glm::vec4         out;
		out.a = in.a;

		float      min, max, delta;

		min = in.r < in.g ? in.r : in.g;
		min = min < in.b ? min : in.b;

		max = in.r > in.g ? in.r : in.g;
		max = max > in.b ? max : in.b;

		out.b = max;                                // v
		delta = max - min;
		if (delta < 0.00001f)
		{
			out.g = 0;
			out.r = 0; // undefined, maybe nan?
			return out;
		}
		if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
			out.g = (delta / max);                  // s
		}
		else {
			// if max is 0, then r = g = b = 0              
			// s = 0, v is undefined
			out.g = 0.0;
			out.r = NAN;                            // its now undefined
			return out;
		}
		if (in.r >= max)                           // > is bogus, just keeps compilor happy
			out.r = (in.g - in.b) / delta;        // between yellow & magenta
		else
			if (in.g >= max)
				out.r = 2.0f + (in.b - in.r) / delta;  // between cyan & yellow
			else
				out.r = 4.0f + (in.r - in.g) / delta;  // between magenta & cyan

		out.r *= 60.0f;                              // degrees

		if (out.r < 0.0f)
			out.r += 360.0f;

		return out;
	}
}
