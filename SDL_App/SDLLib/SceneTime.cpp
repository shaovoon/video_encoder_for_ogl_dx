/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#include "SceneTime.h"

namespace Library
{
	SceneTime::SceneTime()
		: mTotalGameTime(0.0f), mElapsedGameTime(0.0f)
	{
	}

	SceneTime::SceneTime(float totalGameTime, float elapsedGameTime)
		: mTotalGameTime(totalGameTime), mElapsedGameTime(elapsedGameTime)
	{
	}

	float SceneTime::TotalGameTime() const
	{
		return mTotalGameTime;
	}

	void SceneTime::SetTotalGameTime(float totalGameTime)
	{
		mTotalGameTime = totalGameTime;
	}

	float SceneTime::ElapsedGameTime() const
	{
		return mElapsedGameTime;
	}

	void SceneTime::SetElapsedGameTime(float elapsedGameTime)
	{
		mElapsedGameTime = elapsedGameTime;
	}
}