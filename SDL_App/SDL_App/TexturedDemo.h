#pragma once

#include "DrawableSceneComponent.h"
#include "ShaderProgram.h"
#include "Scene.h"
#include "Texture.h"

using namespace Library;

namespace Rendering
{
	class TexturedDemo : public DrawableSceneComponent
	{
		RTTI_DECLARATIONS(TexturedDemo, DrawableSceneComponent)

	public:		
		TexturedDemo(Scene& game, Camera& camera);
		~TexturedDemo();

		virtual void Initialize() override;
		virtual void Draw(const SceneTime& gameTime) override;

		virtual void DownloadFiles() override;
		virtual bool OpenFile(const char* file, FileType file_type) override;

	private:
		TexturedDemo();
		TexturedDemo(const TexturedDemo& rhs);
		TexturedDemo& operator=(const TexturedDemo& rhs);

		void CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer);
		void OnKey(int key, int scancode, int action, int mods);

		ShaderProgram mShaderProgram;
		GLint mWorldViewProjectionLocation;
		glm::mat4 mWorldMatrix;
		Texture mColorTexture;
		GLint  mPositionLocation;
		GLint  mTexCoordLocation;
		GLint mSamplerLocation;
		GLuint mVboId;
		GLuint mIndexId;
		std::string mImageFile;
	};
}
