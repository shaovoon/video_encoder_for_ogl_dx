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

namespace Library
{
	class SceneTime;

	class SceneClock
	{
	public:
		SceneClock();

		const float & StartTime() const;
		const float & CurrentTime() const;
		const float & LastTime() const;

		void Reset();
		float GetFrequency() const;
		void GetTime(float & time) const;
		void UpdateGameTime(SceneTime& gameTime);
		void SetPause(bool enable);

#ifdef VIDEO_ENCODER
		void setTimeQuandant(float quandant);
#endif

	private:
		SceneClock(const SceneClock& rhs);
		SceneClock& operator=(const SceneClock& rhs);

		float  mStartTime;
		float  mCurrentTime;
		float  mLastTime;
		float mFrequency;
		float mPauseTime;
		float mAccumulativePauseTime;
		bool mPaused;
#ifdef VIDEO_ENCODER
		float mQuandant;
#endif
	};
}
