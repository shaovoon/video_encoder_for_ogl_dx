#include "BubblyText.h"
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
#include "csv_stream.h"
#include "DownloadSingleton.h"
#include <VariaNetDebugPrint.h>

extern Library::DownloadSingleton gDownloadSingleton;

using namespace glm;

namespace Rendering
{
	RTTI_DEFINITIONS(BubblyText)

		BubblyText::BubblyText(Scene& game, Camera& camera, const std::string& text, bool gen_text, 
			const std::string& bubblyTextImage,
			const std::string& bubblyTextShadowImage,
			const std::string& imageConfig,
			GLfloat fScreenWidth, GLfloat fScreenHeight, bool stencil)
		: DrawableSceneComponent(game, camera)
		, mText(text)
		, mFloatScreenWidth(fScreenWidth*1.49f), mFloatScreenHeight(fScreenHeight*1.49f)
		, mQuadSize(0.2f), mZPos(0.5f), mDim(128)
		, mBubblyTextImage(bubblyTextImage)
		, mBubblyTextShadowImage(bubblyTextShadowImage)
		, mImageConfig(imageConfig)
		, mStencilBufferEnabled(stencil)
		, mWorldViewProjectionLocation(-1), mWorldMatrix()
		, mPositionLocation(-1), mTexCoordLocation(-1), mSamplerLocation(-1), mSwipeLocation(-1), mPenultimateLocation(-1)
	{
#ifdef _WIN32
		if (gen_text)
		{
			std::string text_arg = "\"";
			text_arg += text;
			text_arg += "\"";
			Utility::RunSilent("C:\\Users\\wong\\Documents\\Bitbucket\\App\\SDL_App\\GenBubblyText\\bin\\Debug\\GenBubblyText.exe", text_arg.c_str());
		}
#endif // _WIN32

		ReadConfig(mImageConfig);
		DownloadFiles();
	}

	BubblyText::~BubblyText()
	{
		mShadowInfo.Destroy();
		mInfo.Destroy();
		mTexture.Destroy();
		mShadowTexture.Destroy();
	}

	void BubblyText::Initialize()
	{
		if (mText.empty())
		{
			return;
		}

		{
			QuadInfo quad;
			InitQuad(quad, true);
			mShadowInfo = quad;
		}
		{
			QuadInfo quad;
			InitQuad(quad, false);
			mInfo = quad;
		}

		InitShader();

		SetInitialized(true);
	}

	void BubblyText::InitQuad(QuadInfo &quad, bool isShadow)
	{
		mQuadSize = 1.0f;
		mZPos = 0.02f;

		GLfloat scale = 1.0f;
		GLfloat w = mActualImageWidth / (GLfloat) mDim;
		GLfloat h = mActualImageHeight / (GLfloat) mDim;

		GLfloat left = -w*scale;
		GLfloat top = h*scale;
		GLfloat right = w*scale;
		GLfloat bottom = -h*scale;

		GLfloat& u = w;
		GLfloat& v = h;
		GLfloat vVertices [] = {
			left, top, 0.0f, 0.0f, 0.0f,
			left, bottom, 0.0f, 0.0f, v,
			right, bottom, 0.0f, u, v,
			right, top, 0.0f, u, 0.0f
		};

		GLushort indices [] = { 0, 1, 2, 0, 2, 3 };

		VertexBufferHelper::createVBO(vVertices,
			indices, 4, sizeof(indices) / sizeof(GLushort),
			quad.mVboId, quad.mIndexId, 5, GL_STATIC_DRAW);

		if (isShadow)
		{
			mShadowTexture.LoadSquareImageFile("", mDownloadedBubblyTextShadowImage,
				mDim, mActualImageWidth, mActualImageHeight, false);
		}
		else
		{
			mTexture.LoadSquareImageFile("", mDownloadedBubblyTextImage,
				mDim, mActualImageWidth, mActualImageHeight, false);
		}
	}
	void BubblyText::InitShader()
	{
		const char* vert_shader = R"vert(uniform mat4 WorldViewProjection;
attribute vec3 a_position;
attribute vec2 a_texCoord;
varying vec2 v_texCoord;
void main()
{
    gl_Position = WorldViewProjection * vec4(a_position, 1.0);
    v_texCoord = a_texCoord;
}
)vert";

		const char* frag_shader = R"frag(varying vec2 v_texCoord;
uniform sampler2D s_texture;
uniform float s_swipe;
uniform float s_penultimate;
void main()
{
	vec4 color = texture2D( s_texture, v_texCoord );
	float curr_swipe = v_texCoord.x+(v_texCoord.y*0.4);
	if( curr_swipe > s_swipe)
		color = vec4(0.0,0.0,0.0,0.0);
	else if(curr_swipe >= s_swipe-s_penultimate && curr_swipe <= s_swipe)
	{
		color.a = color.a * (1.0 - (curr_swipe - (s_swipe - s_penultimate))/s_penultimate);
		color.a = clamp(color.a, 0.0, 1.0);
	}
    gl_FragColor = vec4(color.r,color.g,color.b,color.a);
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
	}

	void BubblyText::Draw(const SceneTime& gameTime)
	{
		if (mStencilBufferEnabled)
		{
			glStencilFunc(GL_EQUAL, 1, 0xFF);
			glStencilMask(0x00);
			glDepthMask(GL_TRUE);
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Draw text shadow first.
		//DrawQuad(mShadowInfo, true, gameTime);
		DrawQuad(mInfo, false, gameTime);
		glDisable(GL_BLEND);

		if (mStencilBufferEnabled)
		{
			glDisable(GL_STENCIL_TEST);
		}
	}

	void BubblyText::DrawQuad(QuadInfo &quad, bool isShadow, const SceneTime& gameTime)
	{
		glActiveTexture(GL_TEXTURE0);
		if (isShadow)
		{
			glBindTexture(GL_TEXTURE_2D, mShadowTexture.getTexture());
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, mTexture.getTexture());
		}

		glUseProgram(mShaderProgram.Program());

		glm::mat4x4 mat;
		updatePosition(gameTime, 5, isShadow, mat);
		updateSwipe(gameTime, quad);

		mat4 wvp = mCamera->ViewProjectionMatrix() * mat;
		gl::Send(mWorldViewProjectionLocation, wvp);

		gl::BindBuffers(quad.mVboId, quad.mIndexId);

		gl::AttribPtr(mPositionLocation, 3, 5, 0);
		gl::AttribPtr(mTexCoordLocation, 2, 5, 3);

		gl::Send(mSamplerLocation, 0);

		gl::Send(mSwipeLocation, quad.mSwipe);

		gl::Send(mPenultimateLocation, quad.mPenultimate);

		// Draw 
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

		gl::UnbindBuffers();
	}


	void BubblyText::CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer)
	{
	}
	void BubblyText::updateSwipe(const SceneTime &gameTime, QuadInfo& quad)
	{
		if (quad.mSwipeStartTime == 0.0f)
			quad.mSwipeStartTime = gameTime.TotalGameTime();
		GLfloat mod2 = gameTime.TotalGameTime() - quad.mSwipeStartTime;
		GLfloat quantum = 1.5f;
		if (!quad.mSwipeCompleted&&quad.mSwipe <= (1.01f + quad.mPenultimate + 0.4f))
		{
			mod2 /= quantum;
			quad.mSwipe = (mod2 + quad.mSwipeSpeed) / quantum;

			quad.mSwipeSpeed += 0.007f;
		}
		else
		{
			quad.mSwipeCompleted = true;
			quad.mSwipe = 1.0f + quad.mPenultimate + 0.4f;
		}

	}
	void BubblyText::updatePosition(const SceneTime &gameTime, int pos, bool isShadow, glm::mat4x4& mat)
	{
		GLfloat margin = 0.3f;
		GLfloat width = (mFloatScreenWidth - (margin*2.0f)) / mText.size();

		GLfloat mod2 = fmod(gameTime.TotalGameTime(), 4.0f);
		GLfloat mod4 = fmod(gameTime.TotalGameTime(), 8.0f);
		bool to_right = mod4 > 4.0f;
		GLfloat delta = 0.0f;
		if (to_right)
			delta = 0.02f*(mod2 - 2.0f);
		else
			delta = 0.02f*((4.0f - mod2) - 2.0f);

		static GLfloat delta_y = 0.0f;
		if (delta>0.0f)
			delta_y += 0.0003f;
		else
			delta_y -= 0.0003f;

		GLfloat x_pos = (pos * width + margin) - (mFloatScreenWidth/2.0f);
		GLfloat y_pos = -0.12f;
		GLfloat z_pos = mZPos;

		if (isShadow)
		{
			x_pos += 0.012f;
			y_pos -= 0.012f;
			z_pos -= 0.003f;
		}

		mat = glm::translate(glm::vec3(x_pos, y_pos, z_pos));
	}

	void BubblyText::ReadConfig(const std::string& config)
	{
		capi::csv::istringstream ifs(config.c_str());
		ifs.set_delimiter(',', "#comma;");
		int r = 0, g = 0, b = 0;
		if (ifs.read_line())
		{
			ifs >> mDim >> mActualImageWidth >> mActualImageHeight;
		}
	}

	void BubblyText::DownloadFiles()
	{
		std::string srcFolder = Utility::CombineFolder(gDownloadSingleton.getSrcFolder(), "Generated");
		gDownloadSingleton.DownloadFile(this, srcFolder, mBubblyTextImage,
			Library::DownloadableComponent::FileType::IMAGE_FILE);
		gDownloadSingleton.DownloadFile(this, srcFolder, mBubblyTextShadowImage,
			Library::DownloadableComponent::FileType::SHADOW_IMAGE_FILE);
	}
	bool BubblyText::OpenFile(const char* file, FileType file_type)
	{
		if (file_type == Library::DownloadableComponent::FileType::IMAGE_FILE)
		{
			mDownloadedBubblyTextImage = file;
			return true;
		}
		if (file_type == Library::DownloadableComponent::FileType::SHADOW_IMAGE_FILE)
		{
			mDownloadedBubblyTextShadowImage = file;
			return true;
		}

		return false;
	}


}