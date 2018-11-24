/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#pragma once

#include "Common.h"
#include "DrawableSceneComponent.h"
#include "ShaderProgram.h"
#include "VertexDeclarations.h"
#include "Texture.h"

namespace Library
{
	class DiffuseModelList : public DrawableSceneComponent
    {
		RTTI_DECLARATIONS(DiffuseModelList, DrawableSceneComponent)
		typedef VertexPositionTextureNormal VertFormat;

    public:
		DiffuseModelList(Scene& game, Camera& camera, const std::string& srcImageFile, const std::vector<std::string>& srcObjFiles, const std::string& srcMtlFile);
		~DiffuseModelList();

		virtual void Initialize() override;
		virtual void Draw(const SceneTime& gameTime) override;
		void Draw(const SceneTime& gameTime, size_t index);

		virtual void DownloadFiles() override;
		virtual bool OpenFile(const char* file, FileType file_type) override;
		virtual void UpdatePosition(const SceneTime& gameTime, glm::mat4x4& mat);

		//virtual void Initialize(GLuint vertexArrayObject) override;
		virtual void CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer) const;
		void CreateVertexBuffer(VertFormat* vertices, GLuint vertexCount, GLuint& vertexBuffer) const;
		virtual size_t VertexSize() const;

		glm::vec3 getLightColor() const { return mLightColor; }
		void setLightColor(const glm::vec3& val) { mLightColor = val; }

		const size_t getModelCount() const { return mModelVertices.size(); }

	private:
		struct ModelVertices
		{
			ModelVertices() : mVboId(0), mIndexId(0), mIndexCount(0) {}
			ModelVertices(const std::string& srcObjFile) : mVboId(0), mIndexId(0), mIndexCount(0), mSrcObjFile(srcObjFile)
			{
				mDestObjFile.reserve(50); mFile.reserve(50);
			}
			GLuint mVboId;
			GLuint mIndexId;
			size_t mIndexCount;
			std::string mSrcObjFile;
			std::string mDestObjFile;
			std::string mFile;
		};

		ShaderProgram mShaderProgram;
		GLint mWorldViewProjectionLocation;
		GLint mModelLocation;
		glm::mat4 mWorldMatrix;
		Texture mColorTexture;
		GLint  mPositionLocation;
		GLint  mTexCoordLocation;
		GLint  mNormalLocation;
		GLint mSamplerLocation;
		glm::vec3 mLightColor;
		glm::vec3 mLightPosition;
		std::string mSrcImageFile;
		std::string mSrcMtlFile;
		std::string mDestImageFile;
		std::string mDestMtlFile;

		GLint mLightColorLocation;
		GLint mLightPositionLocation;
		GLint mTransInverseModelLocation;
		std::vector<ModelVertices> mModelVertices;
	};
}