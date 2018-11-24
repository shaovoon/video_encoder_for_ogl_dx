/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#include "StencilQuad.h"
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
	RTTI_DEFINITIONS(StencilQuad)

	StencilQuad::StencilQuad(Scene& game, Camera& camera, Stage& stage)
		: DrawableSceneComponent(game, camera), mShaderProgram(), 
		mWorldViewProjectionLocation(-1), mWorldMatrix(), 
		mPositionLocation(-1), mVboId(-1), mIndexId(-1),
		mScale(0.25f), mShiftDown(-(4.8308f * mScale) * 0.5f), mScaleX(0.34f), 
		mShiftFront(0.8f), mAniTime(1.4f),
		mStage(stage), mStartTime(0.0f), mStarted(false), mInProgress(false)
	{
		DownloadFiles();
	}

	StencilQuad::~StencilQuad()
	{
		glDeleteBuffers(1, &mVboId);
		glDeleteBuffers(1, &mIndexId);
	}

	void StencilQuad::Initialize()
	{
		LOGFUNCTION("StencilQuad", "Initialize");

		GLfloat ZCoord = 0.109f;
		GLfloat vVertices[] = {
			-0.9344f, 4.8308f, ZCoord,
			-0.9344f, 0.07f, ZCoord,
			0.978f, 0.07f, ZCoord,
			0.978f, 4.8308f, ZCoord
		};

		GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

		VertexBufferHelper::createVBO(vVertices,
			indices, 4, sizeof(indices) / sizeof(GLushort),
			mVboId, mIndexId, 5, GL_STATIC_DRAW);

		const char* vert_shader = R"vert(uniform mat4 WorldViewProjection;
attribute vec3 a_position;
void main()
{
    gl_Position = WorldViewProjection * vec4(a_position, 1.0);
})vert";

		const char* frag_shader = R"frag(
void main()
{
    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
})frag";

		// Build the shader program
		std::vector<ShaderSourceDefinition> shaders;
		shaders.push_back(ShaderSourceDefinition(GL_VERTEX_SHADER, vert_shader));
		shaders.push_back(ShaderSourceDefinition(GL_FRAGMENT_SHADER, frag_shader));
		try
		{
			mShaderProgram.BuildProgramFromSource(shaders);

			mWorldViewProjectionLocation = mShaderProgram.GetUniLoc("WorldViewProjection");
			mPositionLocation = mShaderProgram.GetAttLoc("a_position");
		}
		catch (SceneException& e)
		{
			throw SceneException(MY_FUNC, e.GetError().c_str());
		}

		SetInitialized(true);
	}

	void StencilQuad::Draw(const SceneTime& gameTime)
	{
		if (mInProgress == false)
			return;

		if (mStarted == false && mInProgress)
		{
			mStarted = true;
			mStartTime = gameTime.TotalGameTime();
		}
		SET_PROJECTION(Library::Projection::PERSPECTIVE);

		glUseProgram(mShaderProgram.Program());

		//glEnable(GL_STENCIL_TEST);
		if (mStage == Stage::Stencil1)
		{
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
		}
		else if (mStage == Stage::Stencil2)
		{
			glStencilFunc(GL_ALWAYS, 2, 0xFF);
		}
		glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
		glStencilMask(0xFF);
		glClear(GL_STENCIL_BUFFER_BIT);

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);

		glm::mat4x4 mat;
		UpdateFramePosition(gameTime, mat);

		mat4 wvp = mCamera->ViewProjectionMatrix() * mat;
		gl::Send(mWorldViewProjectionLocation, wvp);

		gl::BindBuffers(mVboId, mIndexId);

		UpdateVertices(gameTime);

		gl::AttribPtr(mPositionLocation, 3, 3, 0);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

		gl::UnbindBuffers();

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
	}

	void StencilQuad::CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer)
	{
	}

	void StencilQuad::UpdateVertices(const SceneTime& gameTime)
	{
		GLfloat elapsed = gameTime.TotalGameTime() - mStartTime;

		if (elapsed > mAniTime)
			elapsed = mAniTime;

		GLfloat scale = elapsed / mAniTime;

		GLfloat end_x = (0.9344f + 0.978f) * scale - 0.9344f;
		GLfloat end_y = 4.8308f - (4.8308f - 0.07f) * scale;

		GLfloat ZCoord = 0.109f;
		GLfloat vVertices [] = {
			-0.9344f, 4.8308f, ZCoord,
			-0.9344f, end_y, ZCoord,
			end_x, end_y, ZCoord,
			end_x, 4.8308f, ZCoord
		};

		VertexBufferHelper::updateVBO((GLfloat*) vVertices, sizeof(vVertices));
	}
	void StencilQuad::UpdateFramePosition(const SceneTime& gameTime, glm::mat4x4& mat)
	{
		GLfloat elapsed = gameTime.TotalGameTime() - mStartTime;
		GLfloat scale_up = 1.0f;
		if (elapsed > mAniTime + 1.0f && elapsed < mAniTime + 2.5f)
		{
			scale_up = (elapsed - mAniTime - 1.0f) * 2.0f;
			scale_up += 1.0f;
		}
		else if (elapsed > mAniTime + 2.5f)
		{
			scale_up = 1.5f * 2.0f;
			scale_up += 1.0f;
		}

		glm::mat4x4 s_mat = glm::scale(glm::vec3(mScaleX, mScale, mScale));
		glm::mat4x4 t_mat = glm::translate(glm::vec3(0.0f, mShiftDown, mShiftFront));
		glm::mat4x4 s_mat2 = glm::scale(glm::vec3(scale_up, scale_up, 1.0f));

		mat = s_mat2 * t_mat * s_mat;
	}

	void StencilQuad::updatePosition(const SceneTime &gameTime, glm::mat4x4& mat)
	{
		GLfloat mod16 = fmod(gameTime.TotalGameTime(), 16.0f);

		GLfloat delta_x = 0.0f;
		GLfloat delta_y = 0.0f;
		GLfloat scale_x = 0.02f;
		GLfloat width_x = 0.08f;
		GLfloat scale_y = 0.016f;
		GLfloat width_y = 0.064f;

		if (mod16 >= 0.0f&&mod16 < 4.0f)
		{
			GLfloat x = mod16*scale_x;
			delta_x = -width_x+x;
			GLfloat y = mod16*scale_y;
			delta_y = -y;
		}
		else if (mod16 >= 4.0f&&mod16 < 8.0f)
		{
			GLfloat x = (mod16 - 4.0f)*scale_x;
			delta_x = x;
			GLfloat y = (mod16 - 4.0f)*scale_y;
			delta_y = -width_y + y;
		}
		else if (mod16 >= 8.0f&&mod16 < 12.0f)
		{
			GLfloat x = (mod16 - 8.0f)*scale_x;
			delta_x = width_x - x;
			GLfloat y = (mod16 - 8.0f)*scale_y;
			delta_y = y;
		}
		else if (mod16 >= 12.0f&&mod16 < 16.0f)
		{
			GLfloat x = (mod16 - 12.0f)*scale_x;
			delta_x = -x;
			GLfloat y = (mod16 - 12.0f)*scale_y;
			delta_y = width_y - y;
		}

		mat = glm::translate(glm::vec3(delta_x, delta_y, 0.0f));
	}

	void StencilQuad::DownloadFiles()
	{
	}
	bool StencilQuad::OpenFile(const char* file, FileType file_type)
	{
		return true;
	}


}