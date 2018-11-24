#include "PaintStroke.h"
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
	RTTI_DEFINITIONS(PaintStroke)

	PaintStroke::PaintStroke(Scene& game, Camera& camera)
		: DrawableSceneComponent(game, camera), mShaderProgram(), 
		mWorldViewProjectionLocation(-1), mWorldMatrix(), 
		mPositionLocation(-1), mTexCoordLocation(-1), mSamplerLocation(-1),
		mSwipeLocation(-1), mPenultimateLocation(-1),
		mVboId(-1), mIndexId(-1), mSwipe(0.0f), mSwipeCompleted(false),
		mSwipeStartTime(0.0f), mSwipeSpeed(0.03f), mPenultimate(0.2f)
	{
		DownloadFiles();
	}

	PaintStroke::~PaintStroke()
	{
		mColorTexture.Destroy();
		glDeleteBuffers(1, &mVboId);
		glDeleteBuffers(1, &mIndexId);
	}

	void PaintStroke::Initialize()
	{
		GLfloat vVertices[] = {
			-0.5f, 0.5f, 0.0f, 0.0f, 0.0f,
			-0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
			0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
			0.5f, 0.5f, 0.0f, 1.0f, 0.0f
		};

		GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

		VertexBufferHelper::createVBO(vVertices,
			indices, 4, sizeof(indices) / sizeof(GLushort),
			mVboId, mIndexId, 5, GL_STATIC_DRAW);

		const char* vert_shader = R"vert(uniform mat4 WorldViewProjection;
attribute vec3 a_position;
attribute vec2 a_texCoord;
varying vec2 v_texCoord;
void main()
{
    gl_Position = WorldViewProjection * vec4(a_position, 1.0);
    v_texCoord = a_texCoord;
})vert";

		const char* frag_shader = R"frag(varying vec2 v_texCoord;
uniform sampler2D s_texture;
uniform float s_swipe;
uniform float s_penultimate;
void main()
{
	vec4 color = texture2D( s_texture, v_texCoord );
	float curr_swipe = v_texCoord.x+(v_texCoord.y*0.4);
	if( curr_swipe > s_swipe)
		color = vec4(0.08235, 0.09803, 0.14117, 0.0);
	if(curr_swipe >= s_swipe-s_penultimate && curr_swipe <= s_swipe && color.a > 0.8)
	{
		color.a = 1.0 - (curr_swipe - (s_swipe - s_penultimate))/s_penultimate;
		color.a = clamp(color.a, 0.0, 1.0);
	}
	vec4 alpha = vec4(color.a);

    gl_FragColor = alpha*vec4(color.r,color.g,color.b,color.a);
    //gl_FragColor = color;
}
)frag";

		// Build the shader program
		std::vector<ShaderSourceDefinition> shaders;
		shaders.push_back(ShaderSourceDefinition(GL_VERTEX_SHADER, vert_shader));
		shaders.push_back(ShaderSourceDefinition(GL_FRAGMENT_SHADER, frag_shader));
		try
		{
			mShaderProgram.BuildProgramFromSource(shaders);

			mWorldViewProjectionLocation = mShaderProgram.GetUniLoc("WorldViewProjection");
			// Load the texture
			if (mColorTexture.LoadImageFile(mImageFile, false, false) == 0)
			{
				std::string err = "texture load failed: ";
				err += mImageFile;
				throw SceneException(err.c_str());
			}

			mPositionLocation = mShaderProgram.GetAttLoc("a_position");
			mTexCoordLocation = mShaderProgram.GetAttLoc("a_texCoord");
			mSamplerLocation = mShaderProgram.GetUniLoc("s_texture");
			mSwipeLocation = mShaderProgram.GetUniLoc("s_swipe");
			mPenultimateLocation = mShaderProgram.GetUniLoc("s_penultimate");
		}
		catch (SceneException& e)
		{
			throw SceneException(MY_FUNC, e.GetError().c_str());
		}

		SetInitialized(true);
	}

	void PaintStroke::Draw(const SceneTime& gameTime)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mColorTexture.getTexture());

		glUseProgram(mShaderProgram.Program());

		mat4 wvp = mCamera->ViewProjectionMatrix() * mWorldMatrix;
		gl::Send(mWorldViewProjectionLocation, wvp);

		gl::BindBuffers(mVboId, mIndexId);

		updateSwipe(gameTime);

		gl::AttribPtr(mPositionLocation, 3, 5, 0);
		gl::AttribPtr(mTexCoordLocation, 2, 5, 3);

		gl::Send(mSamplerLocation, 0);

		gl::Send(mSwipeLocation, mSwipe);

		gl::Send(mPenultimateLocation, mPenultimate);

		// Draw 
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

		gl::UnbindBuffers();
		glDisable(GL_BLEND);
	}

	void PaintStroke::CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer)
	{
	}
	void PaintStroke::updateSwipe(const SceneTime &gameTime)
	{
		if (mSwipeStartTime == 0.0f)
			mSwipeStartTime = gameTime.TotalGameTime();
		GLfloat mod2 = gameTime.TotalGameTime() - mSwipeStartTime;
		GLfloat quantum = 1.5f;
		if (!mSwipeCompleted&&mSwipe <= (1.01f + mPenultimate + 0.4f))
		{
			mod2 /= quantum;
			mSwipe = (mod2 + mSwipeSpeed) / quantum;

			mSwipeSpeed += 0.007f;
		}
		else
		{
			mSwipeCompleted = true;
			mSwipe = 1.0f + mPenultimate + 0.4f;
		}

	}

	void PaintStroke::DownloadFiles()
	{
		std::string srcFolder = Utility::CombineFolder(gDownloadSingleton.getSrcFolder(), "Images");

		gDownloadSingleton.DownloadFile(this, srcFolder, "PurpleStroke.png",
			Library::DownloadableComponent::FileType::IMAGE_FILE_POW_OF_2);
	}
	bool PaintStroke::OpenFile(const char* file, FileType file_type)
	{
		if (file_type == Library::DownloadableComponent::FileType::IMAGE_FILE_POW_OF_2)
		{
			mImageFile = file;
			return true;
		}
		return false;
	}


}