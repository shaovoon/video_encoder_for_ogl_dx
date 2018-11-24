/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#pragma once

#include "Camera.h"
#include "Scene.h"

namespace Library
{
    class FirstPersonCamera : public Camera
    {
        RTTI_DECLARATIONS(FirstPersonCamera, Camera)

    public:
        FirstPersonCamera(Scene& game);
        FirstPersonCamera(Scene& game, float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance);

        virtual ~FirstPersonCamera();

        float& MouseSensitivity();
        float& RotationRate();
        float& MovementRate();		
        
        virtual void Initialize() override;
        virtual void Update(const SceneTime& gameTime) override;

		virtual void DownloadFiles() override;
		virtual bool OpenFile(const char* file, FileType file_type) override;

        static const float DefaultMouseSensitivity;
        static const float DefaultRotationRate;
        static const float DefaultMovementRate;        

    protected:
        float mMouseSensitivity;
        float mRotationRate;
        float mMovementRate;

    private:
        FirstPersonCamera(const FirstPersonCamera& rhs);
        FirstPersonCamera& operator=(const FirstPersonCamera& rhs);

		double mLastCursorX;
		double mLastCursorY;
    };
}

