#pragma once

#include "DrawableSceneComponent.h"
#include "ShaderProgram.h"
#include "Scene.h"
#include "Texture.h"

using namespace Library;

namespace Rendering
{
	class PhotoFade : public DrawableSceneComponent
	{
		RTTI_DECLARATIONS(PhotoFade, DrawableSceneComponent)

	public:		
		PhotoFade(Scene& game, Camera& camera, GLfloat fScreenWidth, GLfloat fScreenHeight);
		~PhotoFade();

		void SetTexturePtr1(Texture* pTexture1);  // call it before Initialize
		void SetTexturePtr2(Texture* pTexture2);  // call it before Initialize

		virtual void Initialize() override;
		virtual void Draw(const SceneTime& gameTime) override;

		virtual void DownloadFiles() override;
		virtual bool OpenFile(const char* file, FileType file_type) override;

	private:
		PhotoFade();
		PhotoFade(const PhotoFade& rhs);
		PhotoFade& operator=(const PhotoFade& rhs);

		void CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer);
		void OnKey(int key, int scancode, int action, int mods);

		ShaderProgram mShaderProgram;
		GLint mWorldViewProjectionLocation;
		glm::mat4 mWorldMatrix;
		GLint mFirstTextureIsFrontLocation;
		bool mFirstTextureIsFront;
		GLint mAlphaLocation;
		GLfloat mAlpha;
		std::string mImage1Path;
		std::string mImage2Path;
		Texture* mpTexture1;
		Texture* mpTexture2;
		GLint  mPositionLocation;
		GLint  mTexCoordLocation1;
		GLint  mTexCoordLocation2;
		GLint mSamplerLocation1;
		GLint mSamplerLocation2;
		GLuint mVboId;
		GLuint mIndexId;
		glm::vec4 mBlendColor;
		GLint  mBlendColorLocation;
		GLfloat mFloatScreenWidth;
		GLfloat mFloatScreenHeight;
		std::string mVertexShaderFile;
		std::string mFragmentShaderFile;
	};
}
