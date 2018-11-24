/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "FirstPersonCamera.h"
#include "Scene.h"
#include "SceneTime.h"
#include "VectorHelper.h"

using namespace glm;

namespace Library
{
    RTTI_DEFINITIONS(FirstPersonCamera)

    const float FirstPersonCamera::DefaultRotationRate = 1.0f;
    const float FirstPersonCamera::DefaultMovementRate = 10.0f;
    const float FirstPersonCamera::DefaultMouseSensitivity = 3.0f;

    FirstPersonCamera::FirstPersonCamera(Scene& game)
        : Camera(game),
          mMouseSensitivity(DefaultMouseSensitivity), mRotationRate(DefaultRotationRate), mMovementRate(DefaultMovementRate),
		  mLastCursorX(0.0), mLastCursorY(0.0)
    {
		DownloadFiles();
    }

    FirstPersonCamera::FirstPersonCamera(Scene& game, float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance)
        : Camera(game, fieldOfView, aspectRatio, nearPlaneDistance, farPlaneDistance),
          mMouseSensitivity(DefaultMouseSensitivity), mRotationRate(DefaultRotationRate), mMovementRate(DefaultMovementRate),
		  mLastCursorX(0.0), mLastCursorY(0.0)
    {
		DownloadFiles();
	}

    FirstPersonCamera::~FirstPersonCamera()
    {
    }

    float&FirstPersonCamera:: MouseSensitivity()
    {
        return mMouseSensitivity;
    }


    float& FirstPersonCamera::RotationRate()
    {
        return mRotationRate;
    }

    float& FirstPersonCamera::MovementRate()
    {
        return mMovementRate;
    }

    void FirstPersonCamera::Initialize()
    {
        Camera::Initialize();

		int x = 0;
		int y = 0;
		SDL_GetMouseState(&x, &y);
		mLastCursorX = x;
		mLastCursorY = y;
	}

    void FirstPersonCamera::Update(const SceneTime& gameTime)
    {
        Camera::Update(gameTime);
    }

	void FirstPersonCamera::DownloadFiles() // empty right now but will download the camera config in the future.
	{
	}
	bool FirstPersonCamera::OpenFile(const char* file, FileType file_type)
	{
		return true;
	}

}
