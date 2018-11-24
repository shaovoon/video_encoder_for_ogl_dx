#include "TexturedDemo.h"
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
	RTTI_DEFINITIONS(TexturedDemo)

	TexturedDemo::TexturedDemo(Scene& game, Camera& camera)
		: DrawableSceneComponent(game, camera), mShaderProgram(), 
		mWorldViewProjectionLocation(-1), mWorldMatrix(), 
		mPositionLocation(-1), mTexCoordLocation(-1), mSamplerLocation(-1),
		mVboId(-1), mIndexId(-1)
	{
		DownloadFiles();
	}

	TexturedDemo::~TexturedDemo()
	{
		mColorTexture.Destroy();
		glDeleteBuffers(1, &mVboId);
		glDeleteBuffers(1, &mIndexId);
	}

	void TexturedDemo::Initialize()
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
void main()
{
    gl_FragColor = texture2D( s_texture, v_texCoord );
})frag";

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
		}
		catch (SceneException& e)
		{
			throw SceneException(MY_FUNC, e.GetError().c_str());
		}

		SetInitialized(true);
	}

	void TexturedDemo::Draw(const SceneTime& gameTime)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mColorTexture.getTexture());

		glUseProgram(mShaderProgram.Program());

		mat4 wvp = mCamera->ViewProjectionMatrix() * mWorldMatrix;
		gl::Send(mWorldViewProjectionLocation, wvp);

		gl::BindBuffers(mVboId, mIndexId);

		gl::AttribPtr(mPositionLocation, 3, 5, 0);
		gl::AttribPtr(mTexCoordLocation, 2, 5, 3);

		// Set the sampler texture unit to 0
		gl::Send(mSamplerLocation, 0);

		// Draw 
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

		gl::UnbindBuffers();
	}

	void TexturedDemo::CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer)
	{
	}

	void TexturedDemo::DownloadFiles()
	{
		std::string srcFolder = Utility::CombineFolder(gDownloadSingleton.getSrcFolder(), "Images");

		gDownloadSingleton.DownloadFile(this, srcFolder, "quad2.png",
			Library::DownloadableComponent::FileType::IMAGE_FILE_POW_OF_2);
	}
	bool TexturedDemo::OpenFile(const char* file, FileType file_type)
	{
		if (file_type == Library::DownloadableComponent::FileType::IMAGE_FILE_POW_OF_2)
		{
			mImageFile = file;
			return true;
		}
		return false;
	}


}