/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#pragma once

#include "DrawableSceneComponent.h"
#include "ShaderProgram.h"
#include "Scene.h"
#include "Texture.h"
#include "Enums.h"

using namespace Library;

namespace Rendering
{
	class StencilQuad : public DrawableSceneComponent
	{
		RTTI_DECLARATIONS(StencilQuad, DrawableSceneComponent)

	public:		
		StencilQuad(Scene& game, Camera& camera, Stage& stage);
		~StencilQuad();

		virtual void Initialize() override;
		virtual void Draw(const SceneTime& gameTime) override;

		virtual void DownloadFiles() override;
		virtual bool OpenFile(const char* file, FileType file_type) override;

		void UpdateVertices(const SceneTime& gameTime);

		void UpdateFramePosition(const SceneTime& gameTime, glm::mat4x4& mat);

		void updatePosition(const SceneTime &gameTime, glm::mat4x4& mat); // original function

		void Start() { mInProgress = true; }
		void Stop() { mInProgress = false; mStarted = false; }

	private:
		StencilQuad();
		StencilQuad(const StencilQuad& rhs);
		StencilQuad& operator=(const StencilQuad& rhs);

		void CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer);
		void OnKey(int key, int scancode, int action, int mods);

		ShaderProgram mShaderProgram;
		GLint mWorldViewProjectionLocation;
		glm::mat4 mWorldMatrix;
		GLint  mPositionLocation;
		GLint mSamplerLocation;
		GLuint mVboId;
		GLuint mIndexId;

		const GLfloat mScale;
		const GLfloat mShiftDown;
		const GLfloat mScaleX;
		const GLfloat mShiftFront;
		const GLfloat mAniTime;

		Stage& mStage;

		GLfloat mStartTime;
		bool mStarted;
		bool mInProgress;
	};
}
