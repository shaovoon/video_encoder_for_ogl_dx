/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#pragma once

#include "SceneComponent.h"
#include <stack>

namespace Library
{
	class SceneTime;

	enum class Projection
	{
		ORTHOGONAL,
		PERSPECTIVE
	};

	class Camera : public SceneComponent
	{
		RTTI_DECLARATIONS(Camera, SceneComponent)

	public:
		Camera(Scene& game);
		Camera(Scene& game, float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance);

		virtual ~Camera();

		const glm::vec3& Position() const;
		const glm::vec3& Direction() const;
		const glm::vec3& Up() const;
		const glm::vec3& Right() const;

		float AspectRatio() const;
		float FieldOfView() const;
		float NearPlaneDistance() const;
		float FarPlaneDistance() const;

		const glm::mat4& ViewMatrix() const;
		const glm::mat4& ProjectionMatrix() const;
		glm::mat4 ViewProjectionMatrix() const;

		virtual void SetPosition(float x, float y, float z);
		virtual void SetPosition(const glm::vec3& position);

		virtual void Reset();
		virtual void Initialize() override;
		virtual void Update(const SceneTime& gameTime) override;
		virtual void UpdateViewMatrix();
		virtual void UpdateProjectionMatrix();
		virtual void ApplyRotation(const glm::mat4& transform);

		static const float DefaultFieldOfView;
		static const float DefaultNearPlaneDistance;
		static const float DefaultFarPlaneDistance;

		void Push(Projection proj);
		void Pop();
		Projection CurrentProjection();

	protected:
		float mFieldOfView;
		float mAspectRatio;
		float mNearPlaneDistance;
		float mFarPlaneDistance;

		glm::vec3 mPosition;
		glm::vec3 mDirection;
		glm::vec3 mUp;
		glm::vec3 mRight;

		glm::mat4 mViewMatrix;
		glm::mat4 mProjectionMatrix;
		std::stack<Projection> mProjectionStack;

	private:
		Camera(const Camera& rhs);
		Camera& operator=(const Camera& rhs);
	};

	struct ProjectionStack
	{
		ProjectionStack(Camera& cam, Projection proj) : mCamera(cam)
		{
			mCamera.Push(proj);
		}
		~ProjectionStack()
		{
			mCamera.Pop();
		}

	private:
		Camera& mCamera;

	};
#define SET_PROJECTION(projection)		Library::ProjectionStack proj_stack(*mCamera, projection);

}

