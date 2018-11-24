/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#include "SceneClock.h"
#include "SceneTime.h"
#include <SDL.h>

namespace Library
{
	SceneClock::SceneClock()
		: mStartTime(0.0), mCurrentTime(0.0), mLastTime(0.0), mFrequency(1000.0f), mPauseTime(0.0f), mAccumulativePauseTime(0.0f), mPaused(false)
#ifdef VIDEO_ENCODER
		, mQuandant(0.033333)
#endif
	{
		Reset();
	}

	const float & SceneClock::StartTime() const
	{
		return mStartTime;
	}

	const float & SceneClock::CurrentTime() const
	{
		return mCurrentTime;
	}

	const float & SceneClock::LastTime() const
	{
		return mLastTime;
	}

	void SceneClock::Reset()
	{
		GetTime(mStartTime);
		mCurrentTime = mStartTime;
		mLastTime = mCurrentTime;
	}

	float SceneClock::GetFrequency() const
	{
		return mFrequency;
	}

	void SceneClock::GetTime(float& time) const
	{
#ifdef VIDEO_ENCODER
		time += mQuandant; 
#else
		time = SDL_GetTicks() / mFrequency;
#endif
	}

	void SceneClock::UpdateGameTime(SceneTime& gameTime)
	{
		GetTime(mCurrentTime);
		mCurrentTime -= mAccumulativePauseTime;
		gameTime.SetTotalGameTime((mCurrentTime - mStartTime));
		gameTime.SetElapsedGameTime((mCurrentTime - mLastTime));

		mLastTime = mCurrentTime;
	}

	void SceneClock::SetPause(bool enable)
	{
		if (enable&&mPaused == false)
		{
			GetTime(mPauseTime);
		}
		else if (enable == false && mPaused)
		{
			float currTime = 0.0f;
			GetTime(currTime);
			float pause_time = currTime - mPauseTime;
			mAccumulativePauseTime += pause_time;
		}
		mPaused = enable;
}

#ifdef VIDEO_ENCODER
	void SceneClock::setTimeQuandant(float quandant)
	{
		mQuandant = quandant;
	}
#endif

}