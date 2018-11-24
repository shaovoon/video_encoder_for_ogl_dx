/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#pragma once

#include "Common.h"
#include "SceneComponent.h"

namespace Library
{
	class Light : public SceneComponent
	{
		RTTI_DECLARATIONS(Light, SceneComponent)

	public:
		Light(Scene& game);
		virtual ~Light();

		const glm::vec4& Color() const;
		void SetColor(float r, float g, float b, float a);
		void SetColor(const glm::vec4& color);

	protected:
		glm::vec4 mColor;
	};
}

