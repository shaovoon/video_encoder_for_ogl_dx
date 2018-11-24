/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "AnimationVariable.h"
#include "SceneException.h"
#include "ColorHelper.h"
#include "Utility.h"
#include "VariaNetDebugPrint.h"

using namespace glm;

namespace Library
{
	AnimationVariable::AnimationVariable()
		: mPerspective(false)
		, mModelIndex(0)
		, mStartTime(0.0f)
		, mStopTime(0.0f)
		, mDelay(0.0f)
		, mInProgress(true)
		, mSubsequent(false)
    {
    }

	AnimationVariable::AnimationVariable(
		bool perspective,
		size_t modelIndex,
		GLfloat startTime,
		GLfloat delay,
		const glm::vec3& initPos,
		const glm::vec3& scale,
		const glm::vec3& translate)
		: mPerspective(perspective)
		, mModelIndex(modelIndex)
		, mStartTime(startTime)
		, mStopTime(0.0f)
		, mDelay(delay)
		, mInitPos(initPos)
		, mScale(scale)
		, mTranslate(translate)
		, mInProgress(true)
		, mSubsequent(false)
	{
	}

	AnimationVariable::AnimationVariable(const AnimationVariable& other)
	{
		mPerspective = other.mPerspective;
		mModelIndex = other.mModelIndex;
		mStartTime = other.mStartTime;
		mStopTime = other.mStopTime;
		mDelay = other.mDelay;
		mInitPos = other.mInitPos;
		m2ndInitPos = other.m2ndInitPos;
		mCurrPos = other.mCurrPos;
		mScale = other.mScale;
		mTranslate = other.mTranslate;
		mInProgress = other.mInProgress;
		mSubsequent = other.mSubsequent;
		mRotationParamList = other.mRotationParamList;
	}

	AnimationVariable& AnimationVariable::operator=(const AnimationVariable& other)
	{
		mPerspective = other.mPerspective;
		mModelIndex = other.mModelIndex;
		mStartTime = other.mStartTime;
		mStopTime = other.mStopTime;
		mDelay = other.mDelay;
		mInitPos = other.mInitPos;
		m2ndInitPos = other.m2ndInitPos;
		mCurrPos = other.mCurrPos;
		mScale = other.mScale;
		mTranslate = other.mTranslate;
		mInProgress = other.mInProgress;
		mSubsequent = other.mSubsequent;
		mRotationParamList = other.mRotationParamList;

		return *this;
	}

	
	AnimationVariable::~AnimationVariable()
	{
	}
}