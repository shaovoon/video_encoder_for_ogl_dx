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
	class ModelEffect : public DrawableSceneComponent
    {
		RTTI_DECLARATIONS(ModelEffect, DrawableSceneComponent)
		typedef VertexPositionTextureNormal VertFormat;

    public:
		ModelEffect(Scene& game, Camera& camera, const std::string& srcImageFile, const std::string& srcSpecularImageFile, const std::string& srcObjFile, const std::string& srcMtlFile);
		~ModelEffect();

		virtual void Initialize() override;
		virtual void Draw(const SceneTime& gameTime) override;

		virtual void DownloadFiles() override;
		virtual bool OpenFile(const char* file, FileType file_type) override;
		virtual void UpdatePosition(const SceneTime& gameTime, glm::mat4x4& mat);

		//virtual void Initialize(GLuint vertexArrayObject) override;
		virtual void CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer) const;
		void CreateVertexBuffer(VertFormat* vertices, GLuint vertexCount, GLuint& vertexBuffer) const;
		virtual size_t VertexSize() const;

		glm::vec3 getLightDirection() const { return mLightDirection; }
		void setLightDirection(const glm::vec3& val) { mLightDirection = val; }
		glm::vec4 getLightColor() const { return mLightColor; }
		void setLightColor(const glm::vec4& val) { mLightColor = val; }
	private:
		ShaderProgram mShaderProgram;
		GLint mWorldViewProjectionLocation;
		glm::mat4 mWorldMatrix;
		Texture mColorTexture;
		Texture mSpecularColorTexture;
		GLint  mPositionLocation;
		GLint  mTexCoordLocation;
		GLint  mNormalLocation;
		GLint mSamplerLocation;
		GLint mSpecularSamplerLocation;
		GLuint mVboId;
		GLuint mIndexId;
		size_t mIndexCount;
		std::string mSrcImageFile;
		std::string mSrcSpecularImageFile;
		std::string mSrcObjFile;
		std::string mSrcMtlFile;
		std::string mDestImageFile;
		std::string mDestSpecularImageFile;
		std::string mDestObjFile;
		std::string mDestMtlFile;

		glm::vec3 mLightDirection;
		glm::vec4 mAmbientColor;
		glm::vec4 mLightColor;
		glm::vec3 mCameraPosition;
		glm::vec4 mSpecularColor;
		GLfloat mSpecularPower;

		GLint mLightDirectionLocation;
		GLint mAmbientColorLocation;
		GLint mLightColorLocation;
		GLint mCameraPositionLocation;
		GLint mSpecularColorLocation;
		GLint mSpecularPowerLocation;

	};
}