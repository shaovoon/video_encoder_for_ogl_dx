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
	class StarModel : public DrawableSceneComponent
    {
		RTTI_DECLARATIONS(StarModel, DrawableSceneComponent)
		typedef VertexPositionNormal VertFormat;

    public:
		StarModel(Scene& game, Camera& camera, const glm::vec3& color, const std::string& srcObjFile, const std::string& srcMtlFile);
		~StarModel();

		virtual void Initialize() override;
		virtual void Draw(const SceneTime& gameTime) override;

		virtual void DownloadFiles() override;
		virtual bool OpenFile(const char* file, FileType file_type) override;
		virtual void UpdatePosition(const SceneTime& gameTime, glm::mat4x4& mat);

		//virtual void Initialize(GLuint vertexArrayObject) override;
		virtual void CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer) const;
		void CreateVertexBuffer(VertFormat* vertices, GLuint vertexCount, GLuint& vertexBuffer) const;
		virtual size_t VertexSize() const;

		glm::vec3 getLightColor() const { return mLightColor; }
		void setLightColor(const glm::vec3& val) { mLightColor = val; }

	private:
		ShaderProgram mShaderProgram;
		GLint mWorldViewProjectionLocation;
		GLint mModelLocation;
		glm::mat4 mWorldMatrix;
		GLint  mPositionLocation;
		GLint  mNormalLocation;
		GLint mSamplerLocation;
		GLuint mVboId;
		GLuint mIndexId;
		size_t mIndexCount;

		glm::vec3 mModelColor;
		glm::vec3 mLightColor;
		glm::vec3 mLightPosition;
		std::string mSrcObjFile;
		std::string mSrcMtlFile;
		std::string mDestObjFile;
		std::string mDestMtlFile;

		GLint mModelColorLocation;
		GLint mLightColorLocation;
		GLint mLightPositionLocation;
		GLint mTransInverseModelLocation;
		GLint mViewPosLocation;

	};
}