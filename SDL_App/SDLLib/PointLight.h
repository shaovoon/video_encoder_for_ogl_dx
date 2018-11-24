/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#pragma once

#include "Common.h"
#include "Light.h"

namespace Library
{
	class PointLight : public Light
	{
		RTTI_DECLARATIONS(PointLight, Light)

	public:
		PointLight(Scene& game);
		virtual ~PointLight();

		const glm::vec3& Position() const;
		float Radius() const;

		virtual void SetPosition(float x, float y, float z);
        virtual void SetPosition(const glm::vec3& position);
		virtual void SetRadius(float value);

		static const float DefaultRadius;

	protected:
		glm::vec3 mPosition;
		float mRadius;
	};
}

