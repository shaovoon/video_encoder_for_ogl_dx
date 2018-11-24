/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "DrawableSceneComponent.h"

namespace Library
{
	RTTI_DEFINITIONS(DrawableSceneComponent)

	DrawableSceneComponent::DrawableSceneComponent()
		: SceneComponent(), mVisible(true), mCamera(nullptr)
	{
	}

	DrawableSceneComponent::DrawableSceneComponent(Scene& game)
		: SceneComponent(game), mVisible(true), mCamera(nullptr)
	{
	}

	DrawableSceneComponent::DrawableSceneComponent(Scene& game, Camera& camera)
		: SceneComponent(game), mVisible(true), mCamera(&camera)
	{
	}

	DrawableSceneComponent::~DrawableSceneComponent()
	{
	}

	bool DrawableSceneComponent::Visible() const
	{
		return mVisible;
	}

	void DrawableSceneComponent::SetVisible(bool visible)
	{
		mVisible = visible;
	}

	Camera* DrawableSceneComponent::GetCamera()
	{
		return mCamera;
	}

	void DrawableSceneComponent::SetCamera(Camera* camera)
	{
		mCamera = camera;
	}

	void DrawableSceneComponent::Draw(const SceneTime& gameTime)
	{
	}
}