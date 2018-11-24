/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#pragma once

#include "Common.h"

namespace Library
{
	enum class Axis
	{
		X, Y, Z
	};

	struct RotationParam
	{
		RotationParam() : mInit(false), mStep(0.0f), mRot(0.0f) {}
		RotationParam(GLfloat step, GLfloat rot, const glm::vec3& dimension) : mInit(true), mStep(step), mRot(rot), mDimension(dimension) {}
		RotationParam(const RotationParam& other)
		{
			mInit = other.mInit;
			mStep = other.mStep;
			mRot = other.mRot;
			mDimension = other.mDimension;
		}
		RotationParam(GLfloat step, GLfloat rot, Axis axis) : mInit(true), mStep(step), mRot(rot)
		{
			switch (axis)
			{
			case Axis::X: mDimension = glm::vec3(1.0f, 0.0f, 0.0f); break;
			case Axis::Y: mDimension = glm::vec3(0.0f, 1.0f, 0.0f); break;
			case Axis::Z: mDimension = glm::vec3(0.0f, 0.0f, 1.0f); break;
			}
		}
		RotationParam& operator=(const RotationParam& other)
		{
			mInit = other.mInit;
			mStep = other.mStep;
			mRot = other.mRot;
			mDimension = other.mDimension;
			return *this;
		}

		bool isInit() const { return mInit; }
		void setInit(bool val) { mInit = val; }
		GLfloat getStep() const { return mStep; }
		void setStep(GLfloat val) { mStep = val; }
		GLfloat getRotAngle() const { return mRot; }
		void setRotAngle(GLfloat val) { mRot = val; }
		const glm::vec3& getDimension() const { return mDimension; }
		void setDimension(const glm::vec3& val) { mDimension = val; }
	private:
		bool mInit;
		GLfloat mStep; // step per second
		GLfloat mRot;
		glm::vec3 mDimension;
	};

	class AnimationVariable
    {

    public:
		AnimationVariable();
		AnimationVariable(
			bool perspective,
			size_t modelIndex,
			GLfloat startTime,
			GLfloat delay,
			const glm::vec3& initPos,
			const glm::vec3& scale,
			const glm::vec3& translate);

		AnimationVariable(const AnimationVariable& other);
		AnimationVariable& operator=(const AnimationVariable& other);

		~AnimationVariable();

		bool isPerspective() const { return mPerspective; }
		void enablePerspective(bool val) { mPerspective = val; }

		size_t getModelIndex() const { return mModelIndex; }
		void setModelIndex(size_t val) { mModelIndex = val; }

		GLfloat getStartTime() const { return mStartTime; }
		void setStartTime(GLfloat val) { mStartTime = val; }

		GLfloat getStopTime() const { return mStopTime; }
		void setStopTime(GLfloat val) { mStopTime = val; }

		GLfloat getDelay() const { return mDelay; }
		void setDelay(GLfloat val) { mDelay = val; }

		const glm::vec3& getInitPos() const { return mInitPos; }
		void setInitPos(const glm::vec3& val) { mInitPos = val; }

		const glm::vec3& get2ndInitPos() const { return m2ndInitPos; }
		void set2ndInitPos(const glm::vec3& val) { m2ndInitPos = val; }

		const glm::vec3& getCurrPos() const { return mCurrPos; }
		void setCurrPos(const glm::vec3& val) { mCurrPos = val; }
		void resetCurrPos() { mCurrPos = mInitPos; }

		const glm::vec3& getScale() const { return mScale; }
		void setScale(const glm::vec3& val) { mScale = val; }

		const glm::vec3& getTranslate() const { return mTranslate; }
		void setTranslate(const glm::vec3& val) { mTranslate = val; }

		bool isInProgress() const { return mInProgress; }
		void setInProgress(bool val) { mInProgress = val; }

		bool isSubsequent() const { return mSubsequent; }
		void setSubsequent(bool val) { mSubsequent = val; }

		void reset()
		{
			resetCurrPos();
		}

		void AddRotationParam(const RotationParam& path) { mRotationParamList.push_back(path); }
		size_t GetRotationParamSize() const { return mRotationParamList.size(); }
		RotationParam& GetRotationParam(size_t index) { return mRotationParamList.at(index); }
		void ClearRotationParam() { mRotationParamList.clear(); }

	private:
		bool mPerspective;
		size_t mModelIndex;
		GLfloat mStartTime;
		GLfloat mStopTime;
		GLfloat mDelay; // delay before restart animation.
		glm::vec3 mInitPos;
		glm::vec3 m2ndInitPos;
		glm::vec3 mCurrPos;
		glm::vec3 mScale;
		glm::vec3 mTranslate;
		bool mInProgress;
		bool mSubsequent;
		std::vector<RotationParam> mRotationParamList;
	};
}