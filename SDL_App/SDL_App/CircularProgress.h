#pragma once

#include "DrawableSceneComponent.h"
#include "DownloadableComponent.h"
#include "ShaderProgram.h"
#include "Scene.h"
#include "Texture.h"

using namespace Library;

namespace Rendering
{
	class CircularProgress  : public DrawableSceneComponent
	{
		RTTI_DECLARATIONS(CircularProgress, DrawableSceneComponent)

	public:		
		CircularProgress(Scene& game, Camera& camera, GLfloat fScreenWidth, GLfloat fScreenHeight);
		~CircularProgress();

		virtual void Initialize() override;
		virtual void Draw(const SceneTime& gameTime) override;

		void UpdatePos(const SceneTime& gameTime, glm::mat4x4& mat);

		virtual void DownloadFiles() override;
		virtual bool OpenFile(const char* file, FileType file_type) override;

	private:
		CircularProgress();
		CircularProgress(const CircularProgress& rhs);
		CircularProgress& operator=(const CircularProgress& rhs);

		void CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer);
		void OnKey(int key, int scancode, int action, int mods);
		GLfloat UpdateAngle(const SceneTime &gameTime);

		ShaderProgram mShaderProgram;
		GLint mWorldViewProjectionLocation;
		glm::mat4 mWorldMatrix;
		Texture mColorTexture;
		GLint  mPositionLocation;
		GLint  mTexCoordLocation;
		GLint mSamplerLocation;
		GLuint mVboId;
		GLuint mIndexId;
		GLfloat mFloatScreenWidth;
		GLfloat mFloatScreenHeight;
		std::string mQuadFile;
	};
}
