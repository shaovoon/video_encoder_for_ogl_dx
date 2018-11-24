/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "ModelEffect.h"
#include "SceneException.h"
#include "Mesh.h"
#include "ColorHelper.h"
#include "VertexBufferHelper.h"
#include "Utility.h"
#include "Camera.h"
#include "DownloadSingleton.h"
#include "VariaNetDebugPrint.h"

extern Library::DownloadSingleton gDownloadSingleton;

using namespace glm;

namespace Library
{
	RTTI_DEFINITIONS(ModelEffect)

		ModelEffect::ModelEffect(Scene& game, Camera& camera, const std::string& srcImageFile, const std::string& srcSpecularImageFile, const std::string& srcObjFile, const std::string& srcMtlFile)
	: DrawableSceneComponent(game, camera), mShaderProgram(),
		mWorldViewProjectionLocation(-1), mWorldMatrix(),
		mPositionLocation(-1), mTexCoordLocation(-1), mNormalLocation(-1), mSamplerLocation(-1), mSpecularSamplerLocation(-1),
		mVboId(-1), mIndexId(-1), mIndexCount(0),
		mSrcImageFile(srcImageFile), mSrcSpecularImageFile(srcSpecularImageFile), mSrcObjFile(srcObjFile), mSrcMtlFile(srcMtlFile), 
		mLightDirection(-1.0f, 0.0f, -1.0f), mLightColor(1.0f, 1.0f, 1.0f, 1.0f),
		mLightDirectionLocation(-1), mAmbientColorLocation(-1), mLightColorLocation(-1),
		mCameraPositionLocation(-1), mSpecularColorLocation(-1), mSpecularPowerLocation(-1)
    {
		DownloadFiles();
    }

	ModelEffect::~ModelEffect()
	{
		mColorTexture.Destroy();
		mSpecularColorTexture.Destroy();
		glDeleteBuffers(1, &mVboId);
		glDeleteBuffers(1, &mIndexId);
	}

	void ModelEffect::Initialize()
	{
		LOGFUNCTION("ModelEffect", "Initialize");

		std::string obj_file = mDestObjFile;
		if (mDestObjFile.find(".gz") == mDestObjFile.size() - 3) // gz compressed file
		{
			obj_file = obj_file.substr(0, obj_file.size() - 3);
		}
		if (mDestObjFile.find(".zip") == mDestObjFile.size() - 4) // gz compressed file, ignore the zip ext.
		{
			obj_file = obj_file.substr(0, obj_file.size() - 4);
		}

#ifndef __EMSCRIPTEN__
		if (Utility::FileExists(obj_file.c_str()) == false)
#endif
		{
			Utility::DecompressFile(mDestObjFile.c_str(), obj_file.c_str());
		}

		// Load the model
		std::unique_ptr<Model> model(new Model(*mGame, obj_file, true));

		// Create the vertex and index buffers
		Mesh* mesh = model->Meshes().at(0);
		CreateVertexBuffer(*mesh, mVboId);
		mesh->CreateIndexBuffer(mIndexId);
		mIndexCount = mesh->Indices().size();

		mAmbientColor = model->GetAmbientColor(0);
		mSpecularColor = model->GetSpecularColor(0);
		//mLightColor = model->GetDiffuseColor(0);
		mSpecularPower = model->GetShininess(0);

		const char* vert_shader = R"vert(uniform mat4 WorldViewProjection;
uniform mat4 World;
uniform vec3 LightDirection;

attribute vec4 a_Position;
attribute vec2 a_TextureCoordinate;
attribute vec3 a_Normal;

varying vec2 v_TextureCoordinate;
varying vec3 v_Normal;
varying vec3 v_LightDirection;
varying vec3 v_WorldPosition;

void main()
{	
	gl_Position = WorldViewProjection * a_Position;
	v_TextureCoordinate = a_TextureCoordinate;
	v_Normal = (World * vec4(a_Normal, 0.0)).xyz;
	v_LightDirection = -LightDirection;
	v_WorldPosition = (World * a_Position).xyz;
})vert";

		const char* frag_shader = R"frag(uniform sampler2D ColorTextureSampler;
uniform sampler2D SpecularColorTextureSampler;
uniform vec4 AmbientColor;
uniform vec4 LightColor;
uniform vec3 CameraPosition;
uniform vec4 SpecularColor;
uniform float SpecularPower;

varying vec2 v_TextureCoordinate;
varying vec3 v_Normal;
varying vec3 v_LightDirection;
varying vec3 v_WorldPosition;

void main()
{	
    vec3 lightDirection = normalize(v_LightDirection);
	vec3 viewDirection = normalize(CameraPosition - v_WorldPosition);

	vec3 normal = normalize(v_Normal);
	float n_dot_l = dot(lightDirection, normal);
	vec3 halfVector = normalize(lightDirection + viewDirection);
	float n_dot_h = dot(normal, halfVector);

	vec4 sampledColor = texture2D(ColorTextureSampler, v_TextureCoordinate);
	vec4 sampledSpecularColor = texture2D(SpecularColorTextureSampler, v_TextureCoordinate);
	vec3 ambient = AmbientColor.rgb * sampledColor.rgb;
	vec3 diffuse = clamp(LightColor.rgb * n_dot_l * sampledColor.rgb, 0.0, 1.0);	
	
	// specular = N.H^n with gloss map stored in color texture's alpha channel
	vec3 specular = SpecularColor.rgb * min(pow(clamp(n_dot_h, 0.0, 1.0), SpecularPower), sampledSpecularColor.r);

	vec4 Color;
	Color.rgb = ambient + diffuse + specular;
	Color.a = sampledColor.a;
	gl_FragColor = Color;
})frag";

		// Build the shader program
		std::vector<ShaderSourceDefinition> shaders;
		shaders.push_back(ShaderSourceDefinition(GL_VERTEX_SHADER, vert_shader));
		shaders.push_back(ShaderSourceDefinition(GL_FRAGMENT_SHADER, frag_shader));
		try
		{
			mShaderProgram.BuildProgramFromSource(shaders);

			mWorldViewProjectionLocation = mShaderProgram.GetUniLoc("WorldViewProjection");
			mLightDirectionLocation = mShaderProgram.GetUniLoc("LightDirection");
			mAmbientColorLocation = mShaderProgram.GetUniLoc("AmbientColor");
			mLightColorLocation = mShaderProgram.GetUniLoc("LightColor");
			mCameraPositionLocation = mShaderProgram.GetUniLoc("CameraPosition");
			mSpecularColorLocation = mShaderProgram.GetUniLoc("SpecularColor");
			mSpecularPowerLocation = mShaderProgram.GetUniLoc("SpecularPower");

			// Load the texture
			if (mColorTexture.LoadImageFile(mDestImageFile, false, false) == 0)
			{
				std::string error = "Color texture load failed.";
				error += "mDestImageFile:";
				error += mDestImageFile;
				error += ", mSrcImageFile:";
				error += mSrcImageFile;
				throw SceneException(error.c_str());
			}
			// Load the texture
			if (mSpecularColorTexture.LoadImageFile(mDestSpecularImageFile, false, false) == 0)
			{
				std::string err = "Specular texture load failed: ";
				err += mDestSpecularImageFile;
				throw SceneException(err.c_str());
			}

			mPositionLocation = mShaderProgram.GetAttLoc("a_Position");
			mTexCoordLocation = mShaderProgram.GetAttLoc("a_TextureCoordinate");
			mNormalLocation = mShaderProgram.GetAttLoc("a_Normal");
			mSamplerLocation = mShaderProgram.GetUniLoc("ColorTextureSampler");
			mSpecularSamplerLocation = mShaderProgram.GetUniLoc("SpecularColorTextureSampler");
		}
		catch (SceneException& e)
		{
			throw SceneException(MY_FUNC, e.GetError().c_str());
		}

		SetInitialized(true);
	}

	void ModelEffect::UpdatePosition(const SceneTime& gameTime, glm::mat4x4& mat)
	{
		static GLfloat y_angle = 0.0f;
		glm::mat4x4 s_mat = glm::scale(glm::vec3(0.003f, 0.003f, 0.003f));
		glm::mat4x4 r_mat = glm::rotate(y_angle, glm::vec3(0.0f, 1.0f, 0.0f));
		y_angle += 0.02f;

		mat = r_mat * s_mat;
	}

	void ModelEffect::Draw(const SceneTime& gameTime)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mColorTexture.getTexture());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mSpecularColorTexture.getTexture());

		glUseProgram(mShaderProgram.Program());

		glm::mat4x4 mat;
		UpdatePosition(gameTime, mat);
		mat4 wvp = mCamera->ViewProjectionMatrix() * mat;
		gl::Send(mWorldViewProjectionLocation, wvp);

		glUniform3fv(mLightDirectionLocation, 1, &mLightDirection[0]);
		glUniform4fv(mAmbientColorLocation, 1, &mAmbientColor[0]);
		glUniform4fv(mLightColorLocation, 1, &mLightColor[0]);
		glUniform3fv(mCameraPositionLocation, 1, &mCameraPosition[0]);
		glUniform4fv(mSpecularColorLocation, 1, &mSpecularColor[0]);
		gl::Send(mSpecularPowerLocation, mSpecularPower);

		gl::BindBuffers(mVboId, mIndexId);

		gl::AttribPtr(mPositionLocation, 4, 9, 0);
		gl::AttribPtr(mTexCoordLocation, 2, 9, 4);
		gl::AttribPtr(mNormalLocation, 3, 9, 6);

		gl::Send(mSamplerLocation, 0);
		gl::Send(mSpecularSamplerLocation, 1);

		// Draw 
		glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, 0);

		gl::UnbindBuffers();
	}

	void ModelEffect::CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer) const
	{
		const std::vector<vec3>& sourceVertices = mesh.Vertices();

		std::vector<VertFormat> vertices;
		vertices.reserve(sourceVertices.size());

		const std::vector<glm::vec2>& textureCoordinates = mesh.TextureCoordinates();
		assert(textureCoordinates.size() == sourceVertices.size());

		const std::vector<vec3>& normals = mesh.Normals();
		assert(normals.size() == sourceVertices.size());

		for (size_t i = 0; i < sourceVertices.size(); i++)
		{
			vec3 position = sourceVertices.at(i);
			vec2 uv = (vec2)textureCoordinates[i];
			vec3 normal = normals.at(i);
			vertices.push_back(VertFormat(vec4(position.x, position.y, position.z, 1.0f), uv, normal));
		}

		CreateVertexBuffer(&vertices[0], vertices.size(), vertexBuffer);
	}

	void ModelEffect::CreateVertexBuffer(VertFormat* vertices, GLuint vertexCount, GLuint& vertexBuffer) const
	{
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, VertexSize() * vertexCount, &vertices[0], GL_STATIC_DRAW);
	}

	size_t ModelEffect::VertexSize() const
    {
		return sizeof(VertFormat);
    }

	void ModelEffect::DownloadFiles()
	{
		std::string srcFolder = Utility::CombineFolder(gDownloadSingleton.getSrcFolder(), "Models");

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcImageFile,
			Library::DownloadableComponent::FileType::IMAGE_FILE_POW_OF_2);

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcSpecularImageFile,
			Library::DownloadableComponent::FileType::SPECULAR_IMAGE_FILE_POW_OF_2);

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcObjFile,
			Library::DownloadableComponent::FileType::OBJ_FILE);

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcMtlFile,
			Library::DownloadableComponent::FileType::MTL_FILE);
	}

	bool ModelEffect::OpenFile(const char* file, FileType file_type)
	{
		if (file_type == Library::DownloadableComponent::FileType::IMAGE_FILE_POW_OF_2)
		{
			mDestImageFile = file;
			return true;
		}
		if (file_type == Library::DownloadableComponent::FileType::SPECULAR_IMAGE_FILE_POW_OF_2)
		{
			mDestSpecularImageFile = file;
			return true;
		}
		if (file_type == Library::DownloadableComponent::FileType::OBJ_FILE)
		{
			mDestObjFile = file;
			return true;
		}
		if (file_type == Library::DownloadableComponent::FileType::MTL_FILE)
		{
			mDestMtlFile = file;
			return true;
		}
		return false;
	}

}