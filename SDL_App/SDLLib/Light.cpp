/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "Light.h"
#include "ColorHelper.h"

namespace Library
{
	RTTI_DEFINITIONS(Light)

	Light::Light(Scene& game)
		: SceneComponent(game), mColor(ColorHelper::White)
	{
	}

	Light::~Light()
	{
	}

	const glm::vec4& Light::Color() const
	{
		return mColor;
	}

	void Light::SetColor(float r, float g, float b, float a)
	{
		glm::vec4 color(r, g, b, a);
		SetColor(color);
	}

	void Light::SetColor(const glm::vec4& color)
	{
		mColor = color;
	}
}