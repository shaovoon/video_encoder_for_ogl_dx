#pragma once

#include <map>
#include <vector>
#include <string>
#include <Utility.h>
#include "DrawableSceneComponent.h"
#include "ShaderProgram.h"
#include "Scene.h"
#include "Texture.h"

using namespace Library;

namespace Rendering
{
	class BubblyText : public DrawableSceneComponent
	{
		RTTI_DECLARATIONS(BubblyText, DrawableSceneComponent)

	public:		
		BubblyText(Scene& game, Camera& camera, const std::string& text, bool gen_text, 
			const std::string& bubblyTextImage,
			const std::string& bubblyTextShadowImage,
			const std::string& imageConfig,
			GLfloat fScreenWidth, GLfloat fScreenHeight, bool stencil);
		~BubblyText();

		virtual void Initialize() override;
		virtual void Draw(const SceneTime& gameTime) override;

		virtual void DownloadFiles() override;
		virtual bool OpenFile(const char* file, FileType file_type) override;

		void updatePosition(const SceneTime &gameTime, int pos, bool isShadow, glm::mat4x4& mat);

		const std::string& getText() const { return mText; }
		void setText(const std::string& val) { mText = val; }

	private:
		BubblyText();
		BubblyText(const BubblyText& rhs);
		BubblyText& operator=(const BubblyText& rhs);

		void CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer);
		void OnKey(int key, int scancode, int action, int mods);
		void ReadConfig(const std::string& config);
		void InitShader();

		struct QuadInfo
		{
			QuadInfo() : mVboId(-1), mIndexId(-1), mSwipe(0.0f), mSwipeCompleted(false),
				mSwipeStartTime(0.0f), mSwipeSpeed(0.03f), mPenultimate(0.2f) {}

			void Destroy()
			{
				glDeleteBuffers(1, &mVboId);
				glDeleteBuffers(1, &mIndexId);
			}

			GLuint mVboId;
			GLuint mIndexId;
			GLfloat mSwipe;
			bool mSwipeCompleted;
			GLfloat mSwipeStartTime;
			GLfloat mSwipeSpeed;
			GLfloat mPenultimate;
		};
		void InitQuad(QuadInfo &quad, bool isShadow);
		void DrawQuad(QuadInfo &quad, bool isShadow, const SceneTime& gameTime);
		void updateSwipe(const SceneTime &gameTime, QuadInfo& quad);

		ShaderProgram mShaderProgram;
		Texture mTexture;
		Texture mShadowTexture;
		std::string mText;
		QuadInfo mInfo;
		QuadInfo mShadowInfo;
		GLfloat mFloatScreenWidth;
		GLfloat mFloatScreenHeight;
		GLfloat mQuadSize;
		GLfloat mZPos;
		int mActualImageWidth;
		int mActualImageHeight;
		int mDim;
		std::string mBubblyTextImage;
		std::string mBubblyTextShadowImage;
		std::string mDownloadedBubblyTextImage;
		std::string mDownloadedBubblyTextShadowImage;
		std::string mImageConfig;
		bool mStencilBufferEnabled;

		GLint mWorldViewProjectionLocation;
		glm::mat4 mWorldMatrix;
		GLint  mPositionLocation;
		GLint  mTexCoordLocation;
		GLint mSamplerLocation;
		GLint mSwipeLocation;
		GLint mPenultimateLocation;

	};
}
