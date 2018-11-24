/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#pragma once

namespace Library
{
	class SceneTime
	{
	public:
		SceneTime();
		SceneTime(float totalGameTime, float elapsedGameTime);

		float TotalGameTime() const;
		void SetTotalGameTime(float totalGameTime);

		float ElapsedGameTime() const;
		void SetElapsedGameTime(float elapsedGameTime);

	private:
		float mTotalGameTime;
		float mElapsedGameTime;
	};
}
