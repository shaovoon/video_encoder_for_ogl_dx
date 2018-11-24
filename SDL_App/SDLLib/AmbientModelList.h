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
#include "Enums.h"

namespace Library
{
	class AmbientModelList : public DrawableSceneComponent
    {
		RTTI_DECLARATIONS(AmbientModelList, DrawableSceneComponent)
		typedef VertexPositionTexture VertFormat;

    public:
		AmbientModelList(Scene& game, Camera& camera, const std::string& srcImageFile, const std::string& srcAllFiles, 
			const std::vector<std::string>& srcObjFiles, const std::vector<size_t>& vecFileSize,
			const std::string& srcMtlFile, bool gen_mipmap, Precision precision);
		~AmbientModelList();

		virtual void Initialize() override;
		virtual void Draw(const SceneTime& gameTime) override;
		void Draw(const SceneTime& gameTime, size_t index, glm::mat4x4& mat);

		void DrawPrologue();
		void DrawVertices(const SceneTime& gameTime, size_t index, glm::mat4x4& mat);
		void DrawEpilogue();

		virtual void DownloadFiles() override;
		virtual bool OpenFile(const char* file, FileType file_type) override;
		virtual void UpdatePosition(const SceneTime& gameTime, glm::mat4x4& mat);

		//virtual void Initialize(GLuint vertexArrayObject) override;
		virtual void CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer) const;
		void CreateVertexBuffer(VertFormat* vertices, GLuint vertexCount, GLuint& vertexBuffer) const;
		virtual size_t VertexSize() const;

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
		void LoadModel(const std::string& file);

		ShaderProgram mShaderProgram;
		GLint mWorldViewProjectionLocation;
		glm::mat4 mWorldMatrix;
		Texture mColorTexture;
		GLint  mPositionLocation;
		GLint  mTexCoordLocation;
		GLint mSamplerLocation;
		std::string mSrcImageFile;
		std::string mSrcAllFiles;
		std::vector<std::string> mSrcObjFiles;
		std::vector<size_t> mVecFileSize;
		std::string mSrcMtlFile;
		std::string mDestImageFile;
		std::string mDestAllFiles;
		std::string mDestMtlFile;
		std::vector<ModelVertices> mModelVertices;
		bool mGenMipmap;
		Precision mPrecision;
	};
}