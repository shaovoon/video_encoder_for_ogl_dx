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
	class Camera;

	class DrawableSceneComponent : public SceneComponent
	{
		RTTI_DECLARATIONS(DrawableSceneComponent, SceneComponent)

	public:
		DrawableSceneComponent();
		DrawableSceneComponent(Scene& game);
		DrawableSceneComponent(Scene& game, Camera& camera);
		virtual ~DrawableSceneComponent();

		bool Visible() const;
		void SetVisible(bool visible);

		Camera* GetCamera();
		void SetCamera(Camera* camera);

		virtual void Draw(const SceneTime& gameTime);

	protected:
		bool mVisible;
		Camera* mCamera;

	private:
		DrawableSceneComponent(const DrawableSceneComponent& rhs);
		DrawableSceneComponent& operator=(const DrawableSceneComponent& rhs);
	};
}