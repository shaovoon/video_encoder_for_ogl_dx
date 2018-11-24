/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#include "SceneComponent.h"
#include "SceneTime.h"

namespace Library
{
	RTTI_DEFINITIONS(SceneComponent)

	SceneComponent::SceneComponent()
		: mGame(nullptr), mEnabled(true)
	{
	}

	SceneComponent::SceneComponent(Scene& game)
		: mGame(&game), mEnabled(true)
	{
	}

	SceneComponent::~SceneComponent()
	{
	}

	Scene* SceneComponent::GetGame()
	{
		return mGame;
	}

	void SceneComponent::SetGame(Scene& game)
	{
		mGame = &game;
	}

	bool SceneComponent::Enabled() const
	{
		return mEnabled;
	}

	void SceneComponent::SetEnabled(bool enabled)
	{
		mEnabled = enabled;
	}

	void SceneComponent::Initialize()
	{
	}

	void SceneComponent::Update(const SceneTime& gameTime)
	{
	}
}
