#include "PhotoFade.h"
#include "SceneException.h"
#include "ColorHelper.h"
#include "Camera.h"
#include "Utility.h"
#include "ShaderProgram.h"
#include "VertexDeclarations.h"
#include "VectorHelper.h"
#include "Model.h"
#include "Mesh.h"
#include "VertexBufferHelper.h"
#include "DownloadSingleton.h"

extern Library::DownloadSingleton gDownloadSingleton;

using namespace glm;

namespace Rendering
{
	RTTI_DEFINITIONS(PhotoFade)

		PhotoFade::PhotoFade(Scene& game, Camera& camera, GLfloat fScreenWidth, GLfloat fScreenHeight)
		: DrawableSceneComponent(game, camera), mShaderProgram(), 
		mWorldViewProjectionLocation(-1), mWorldMatrix(), 
		mFirstTextureIsFrontLocation(-1), mFirstTextureIsFront(true),
		mAlphaLocation(-1), mAlpha(1.0f),
		mPositionLocation(-1),
		mTexCoordLocation1(-1), mTexCoordLocation2(-1), 
		mSamplerLocation1(-1), mSamplerLocation2(-1),
		mVboId(-1), mIndexId(-1), mBlendColor(), mBlendColorLocation(-1),
		mFloatScreenWidth(fScreenWidth), mFloatScreenHeight(fScreenHeight)
	{
		DownloadFiles();
	}

	PhotoFade::~PhotoFade()
	{
		glDeleteBuffers(1, &mVboId);
		glDeleteBuffers(1, &mIndexId);
	}

	void PhotoFade::SetTexturePtr1(Texture* pTexture1)
	{
		mpTexture1 = pTexture1;
	}
	void PhotoFade::SetTexturePtr2(Texture* pTexture2)
	{
		mpTexture2 = pTexture2;
	}
	void PhotoFade::Initialize()
	{
		GLfloat half_width = mFloatScreenWidth / 2.0f;
		GLfloat half_height = mFloatScreenHeight / 2.0f;

		GLfloat vVertices[] = {
			-half_width, half_height, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			-half_width, -half_height, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			half_width, -half_height, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			half_width, half_height, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f
		};

		GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

		VertexBufferHelper::createVBO(&vVertices[0],
			&indices[0], 4, sizeof(indices) / sizeof(GLushort),
			mVboId, mIndexId, 7, GL_DYNAMIC_DRAW);

		// Build the shader program
		std::vector<ShaderDefinition> shaders;
		shaders.push_back(ShaderDefinition(GL_VERTEX_SHADER, Library::Utility::ToWideString(mVertexShaderFile)));
		shaders.push_back(ShaderDefinition(GL_FRAGMENT_SHADER, Library::Utility::ToWideString(mFragmentShaderFile)));
		try
		{
			mShaderProgram.BuildProgram(shaders);

			mWorldViewProjectionLocation = mShaderProgram.GetUniLoc("WorldViewProjection");
			mFirstTextureIsFrontLocation = mShaderProgram.GetUniLoc("s_first_texture_is_front");
			mAlphaLocation = mShaderProgram.GetUniLoc("s_alpha");
			mPositionLocation = mShaderProgram.GetAttLoc("a_position");
			mTexCoordLocation1 = mShaderProgram.GetAttLoc("texCoord1");
			mTexCoordLocation2 = mShaderProgram.GetAttLoc("texCoord2");
			mSamplerLocation1 = mShaderProgram.GetUniLoc("s_texture1");
			mSamplerLocation2 = mShaderProgram.GetUniLoc("s_texture2");
			mBlendColorLocation = mShaderProgram.GetUniLoc("s_blend_color");
		}
		catch (SceneException& e)
		{
			throw SceneException(MY_FUNC, e.GetError().c_str());
		}

		SetInitialized(true);
	}

	void PhotoFade::Draw(const SceneTime& gameTime)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mpTexture1->getTexture());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mpTexture2->getTexture());

		glUseProgram(mShaderProgram.Program());

		mat4 wvp = mCamera->ViewProjectionMatrix() * mWorldMatrix;
		gl::Send(mWorldViewProjectionLocation, wvp);

		mBlendColor[0] = 1.0f;
		mBlendColor[1] = 1.0f;
		mBlendColor[2] = 0.0f;
		mBlendColor[3] = 0.2f;

		glUniform4fv(mBlendColorLocation, 1, &mBlendColor[0]);

		gl::Send(mFirstTextureIsFrontLocation, 1);
		gl::Send(mAlphaLocation, 1.0f);

		gl::BindBuffers(mVboId, mIndexId);

		GLfloat width = 2.0f;
		GLfloat height = 1.5f;
		GLfloat half_width = width / 2.0f;
		GLfloat half_height = height / 2.0f;

		auto uv = mpTexture1->getUV();
		GLfloat vVertices [] = {
			-half_width, half_height, 0.0f, uv.leftUV, uv.topUV, 0.0f, 0.0f,
			-half_width, -half_height, 0.0f, uv.leftUV, uv.bottomUV, 0.0f, 1.0f,
			half_width, -half_height, 0.0f, uv.rightUV, uv.bottomUV, 1.0f, 1.0f,
			half_width, half_height, 0.0f, uv.rightUV, uv.topUV, 1.0f, 0.0f
		};

		VertexBufferHelper::updateVBO((GLfloat*) vVertices, sizeof(vVertices));

		gl::AttribPtr(mPositionLocation, 3, 7, 0);
		gl::AttribPtr(mTexCoordLocation1, 2, 7, 3);
		gl::AttribPtr(mTexCoordLocation2, 2, 7, 5);

		gl::Send(mSamplerLocation1, 0);
		gl::Send(mSamplerLocation2, 1);

		// Draw 
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

		gl::UnbindBuffers();
	}

	void PhotoFade::CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer)
	{
	}

	void PhotoFade::DownloadFiles()
	{
		std::string srcFolder = Utility::CombineFolder(gDownloadSingleton.getSrcFolder(), "Shaders");

		gDownloadSingleton.DownloadFile(this, srcFolder, "photo_fade.vert.txt",
			Library::DownloadableComponent::FileType::VERTEX_SHADER);
		gDownloadSingleton.DownloadFile(this, srcFolder, "photo_fade.frag.txt",
			Library::DownloadableComponent::FileType::FRAGMENT_SHADER);
	}
	bool PhotoFade::OpenFile(const char* file, FileType file_type)
	{
		if (file_type == Library::DownloadableComponent::FileType::VERTEX_SHADER)
		{
			mVertexShaderFile = file;
			return true;
		}
		if (file_type == Library::DownloadableComponent::FileType::FRAGMENT_SHADER)
		{
			mFragmentShaderFile = file;
			return true;
		}
		return false;
	}


}