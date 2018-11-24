#pragma once

#include "DrawableSceneComponent.h"
#include "ShaderProgram.h"
#include "Scene.h"
#include "Texture.h"

using namespace Library;

namespace Rendering
{
	class PaintStroke : public DrawableSceneComponent
	{
		RTTI_DECLARATIONS(PaintStroke, DrawableSceneComponent)

	public:		
		PaintStroke(Scene& game, Camera& camera);
		~PaintStroke();

		virtual void Initialize() override;
		virtual void Draw(const SceneTime& gameTime) override;

		virtual void DownloadFiles() override;
		virtual bool OpenFile(const char* file, FileType file_type) override;

	private:
		PaintStroke();
		PaintStroke(const PaintStroke& rhs);
		PaintStroke& operator=(const PaintStroke& rhs);

		void CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer);
		void OnKey(int key, int scancode, int action, int mods);
		void updateSwipe(const SceneTime &gameTime);

		ShaderProgram mShaderProgram;
		GLint mWorldViewProjectionLocation;
		glm::mat4 mWorldMatrix;
		Texture mColorTexture;
		GLint  mPositionLocation;
		GLint  mTexCoordLocation;
		GLint mSamplerLocation;
		GLint mSwipeLocation;
		GLint mPenultimateLocation;
		GLuint mVboId;
		GLuint mIndexId;
		std::string mImageFile;
		GLfloat mSwipe;
		bool mSwipeCompleted;
		GLfloat mSwipeStartTime;
		GLfloat mSwipeSpeed;
		GLfloat mPenultimate;
	};
}
