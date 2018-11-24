/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "Camera.h"
#include "Scene.h"
#include "SceneTime.h"
#include "VectorHelper.h"

using namespace glm;

namespace Library
{
	RTTI_DEFINITIONS(Camera)

	const float Camera::DefaultFieldOfView = 45.0f;
	const float Camera::DefaultNearPlaneDistance = 0.01f;
	const float Camera::DefaultFarPlaneDistance = 1000.0f;

	Camera::Camera(Scene& game)
		: SceneComponent(game),
		mFieldOfView(DefaultFieldOfView), mAspectRatio(game.AspectRatio()), mNearPlaneDistance(DefaultNearPlaneDistance), mFarPlaneDistance(DefaultFarPlaneDistance),
		mPosition(), mDirection(), mUp(), mRight(), mViewMatrix(), mProjectionMatrix()
	{
		Push(Projection::ORTHOGONAL);
		SetInitialized(true);
	}

	Camera::Camera(Scene& game, float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance)
		: SceneComponent(game),
		mFieldOfView(fieldOfView), mAspectRatio(aspectRatio), mNearPlaneDistance(nearPlaneDistance), mFarPlaneDistance(farPlaneDistance),
		mPosition(), mDirection(), mUp(), mRight(), mViewMatrix(), mProjectionMatrix()
	{
		Push(Projection::ORTHOGONAL);
		SetInitialized(true);
	}

	Camera::~Camera()
	{
		Pop();
	}

	const glm::vec3& Camera::Position() const
	{
		return mPosition;
	}

	const glm::vec3& Camera::Direction() const
	{
		return mDirection;
	}

	const glm::vec3& Camera::Up() const
	{
		return mUp;
	}

	const glm::vec3& Camera::Right() const
	{
		return mRight;
	}

	float Camera::AspectRatio() const
	{
		return mAspectRatio;
	}

	float Camera::FieldOfView() const
	{
		return mFieldOfView;
	}

	float Camera::NearPlaneDistance() const
	{
		return mNearPlaneDistance;
	}

	float Camera::FarPlaneDistance() const
	{
		return mFarPlaneDistance;
	}

	const glm::mat4& Camera::ViewMatrix() const
	{
		return mViewMatrix;
	}

	const glm::mat4&Camera::ProjectionMatrix() const
	{
		return mProjectionMatrix;
	}

	glm::mat4 Camera::ViewProjectionMatrix() const
	{
		return mProjectionMatrix * mViewMatrix;
	}

	void Camera::SetPosition(float x, float y, float z)
	{
		mPosition = vec3(x, y, z);
	}

	void Camera::SetPosition(const glm::vec3& position)
	{
		mPosition = position;
	}

	void Camera::Reset()
	{
		mPosition = Vector3Helper::Zero;
		mDirection = Vector3Helper::Forward;
		mUp = Vector3Helper::Up;
		mRight = Vector3Helper::Right;

		UpdateViewMatrix();
	}

	void Camera::Initialize()
	{
		UpdateProjectionMatrix();
		Reset();
	}

	void Camera::Update(const SceneTime& gameTime)
	{
		UpdateViewMatrix();
	}

	void Camera::UpdateViewMatrix()
	{
		vec3 target = mPosition + mDirection;
		mViewMatrix = lookAt(mPosition, target, mUp);
	}

	void Camera::UpdateProjectionMatrix()
	{
		if (CurrentProjection()==Projection::ORTHOGONAL)
		{
			mProjectionMatrix = ortho(-1.0f, 1.0f, -0.75f, 0.75f, mNearPlaneDistance, mFarPlaneDistance);
		}
		else
		{
			mProjectionMatrix = perspective(mFieldOfView, mAspectRatio, mNearPlaneDistance, mFarPlaneDistance);
		}
	}

	void Camera::ApplyRotation(const glm::mat4& transform)
	{
		vec4 direction = transform * vec4(mDirection, 0.0f);
		mDirection = (vec3)normalize(direction);

		vec4 up	= transform * vec4(mUp, 0.0f);
		mUp = (vec3)normalize(up);

		mRight = cross(mDirection, mUp);
		mUp = cross(mRight, mDirection);
	}

	void Camera::Push(Projection proj)
	{ 
		mProjectionStack.push(proj);
		UpdateProjectionMatrix();
	}

	void Camera::Pop() 
	{ 
		if (!mProjectionStack.empty()) 
			mProjectionStack.pop(); 

		UpdateProjectionMatrix();
	}
	Projection Camera::CurrentProjection() 
	{
		if (!mProjectionStack.empty())
			return mProjectionStack.top();

		return Projection::ORTHOGONAL;
	}

}