#include "CircularProgress.h"
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
	RTTI_DEFINITIONS(CircularProgress)

	CircularProgress::CircularProgress(Scene& game, Camera& camera, GLfloat fScreenWidth, GLfloat fScreenHeight)
		: DrawableSceneComponent(game, camera), mShaderProgram(), 
		mWorldViewProjectionLocation(-1), mWorldMatrix(), 
		mPositionLocation(-1), mTexCoordLocation(-1), mSamplerLocation(-1),
		mVboId(-1), mIndexId(-1),
		mFloatScreenWidth(fScreenWidth*1.49f), mFloatScreenHeight(fScreenHeight*1.49f)
	{
		DownloadFiles();
	}

	CircularProgress::~CircularProgress()
	{
		mColorTexture.Destroy();
		glDeleteBuffers(1, &mVboId);
		glDeleteBuffers(1, &mIndexId);
	}

	void CircularProgress::Initialize()
	{
		const GLfloat scale = 0.16f;
		const GLfloat width = 0.5f*scale;

		GLfloat vVertices[] = {
			-width, width, 0.0f, 0.0f, 0.0f,
			-width, -width, 0.0f, 0.0f, 1.0f,
			width, -width, 0.0f, 1.0f, 1.0f,
			width, width, 0.0f, 1.0f, 0.0f
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
void main()
{
    gl_FragColor = texture2D( s_texture, v_texCoord );
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
			if (mColorTexture.LoadImageFile(mQuadFile, false, false) == 0)
			{
				std::string err = "texture load failed: ";
				err += mQuadFile;
				throw SceneException(err.c_str());
			}

			mPositionLocation = mShaderProgram.GetAttLoc("a_position");
			mTexCoordLocation = mShaderProgram.GetAttLoc("a_texCoord");
			mSamplerLocation = mShaderProgram.GetUniLoc("s_texture");
		}
		catch (SceneException& e)
		{
			throw SceneException(MY_FUNC, e.GetError().c_str());
		}

		SetInitialized(true);
	}

	void CircularProgress::Draw(const SceneTime& gameTime)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mColorTexture.getTexture());

		glUseProgram(mShaderProgram.Program());

		glm::mat4x4 mat;
		UpdatePos(gameTime, mat);
		mat4 wvp = mCamera->ViewProjectionMatrix() * mat;
		gl::Send(mWorldViewProjectionLocation, wvp);

		gl::BindBuffers(mVboId, mIndexId);

		gl::AttribPtr(mPositionLocation, 3, 5, 0);
		gl::AttribPtr(mTexCoordLocation, 2, 5, 3);

		gl::Send(mSamplerLocation, 0);

		// Draw 
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

		gl::UnbindBuffers();
		glDisable(GL_BLEND);
	}

	void CircularProgress::UpdatePos(const SceneTime& gameTime, glm::mat4x4& mat)
	{
		GLfloat angle = glm::radians(UpdateAngle(gameTime));

		mat = glm::rotate(angle, glm::vec3(0.0f, 0.0f, 1.0f));
	}

	void CircularProgress::CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer)
	{
	}

	GLfloat CircularProgress::UpdateAngle(const SceneTime &gameTime)
	{
		GLfloat PI_360 = 360.0f;
		//static double start_time = gameTime.TotalGameTime();
		GLfloat total_animated_time = 2.0f;
		GLfloat mod4 = fmod(gameTime.TotalGameTime(), total_animated_time);

		static GLfloat angle = 0.0f;
		angle = PI_360 * mod4;

		return fmod(angle, 360.0f);
	}

	void CircularProgress::DownloadFiles()
	{
		std::string srcFolder = Utility::CombineFolder(gDownloadSingleton.getSrcFolder(), "Images");

		gDownloadSingleton.DownloadFile(this, srcFolder, "confetti_quad.png",
			Library::DownloadableComponent::FileType::IMAGE_FILE);
	}
	bool CircularProgress::OpenFile(const char* file, FileType file_type)
	{
		if (file_type == Library::DownloadableComponent::FileType::IMAGE_FILE)
		{
			mQuadFile = file;
			return true;
		}
		return false;
	}
}