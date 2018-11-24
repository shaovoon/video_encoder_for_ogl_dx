/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "DiffuseModel.h"
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
	RTTI_DEFINITIONS(DiffuseModel)

	DiffuseModel::DiffuseModel(Scene& game, Camera& camera, const std::string& srcImageFile, const std::string& srcObjFile, const std::string& srcMtlFile)
	: DrawableSceneComponent(game, camera), mShaderProgram(),
		mWorldViewProjectionLocation(-1), mModelLocation(-1), mWorldMatrix(),
		mPositionLocation(-1), mNormalLocation(-1),
		mVboId(-1), mIndexId(-1), mIndexCount(0), mLightColor(1.0f, 1.0f, 1.0f), mLightPosition(1.0f,0.2f,1.0f),
		mSrcImageFile(srcImageFile),
		mSrcObjFile(srcObjFile), mSrcMtlFile(srcMtlFile), mLightColorLocation(-1), 
		mLightPositionLocation(-1), mTransInverseModelLocation(-1)
    {
		DownloadFiles();
    }

	DiffuseModel::~DiffuseModel()
	{
		mColorTexture.Destroy();
		glDeleteBuffers(1, &mVboId);
		glDeleteBuffers(1, &mIndexId);
	}

	void DiffuseModel::Initialize()
	{
		LOGFUNCTION("DiffuseModel", "Initialize");

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

		const char* vert_shader = R"vert(uniform mat4 WorldViewProjection;
uniform mat4 s_Model;
uniform vec3 s_LightColor;
uniform vec3 s_LightPosition;
uniform mat4 s_TransInverseModel;

attribute vec3 a_Position;
attribute vec2 a_TextureCoordinate;
attribute vec3 a_Normal;

varying vec2 v_TextureCoordinate;
varying vec3 v_Normal;
varying vec3 v_FragPos;

void main()
{
    gl_Position = WorldViewProjection * vec4(a_Position, 1.0);
	v_FragPos = vec3(s_Model * vec4(a_Position, 1.0));
	v_Normal = mat3(s_TransInverseModel) * a_Normal; 
	v_TextureCoordinate = a_TextureCoordinate;
}
)vert";

		const char* frag_shader = R"frag(
uniform sampler2D ColorTextureSampler;
uniform vec3 s_LightColor;
uniform vec3 s_LightPosition;

varying vec2 v_TextureCoordinate;
varying vec3 v_Normal;
varying vec3 v_FragPos;

void main()
{
	vec3 norm = normalize(v_Normal);
	vec3 lightDir = normalize(s_LightPosition - v_FragPos);  

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * s_LightColor;

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * s_LightColor;

	vec4 sampledColor = texture2D(ColorTextureSampler, v_TextureCoordinate);

	vec3 result = (ambient + diffuse) * sampledColor.rgb;
	gl_FragColor = vec4(result, 1.0);
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
			mModelLocation = mShaderProgram.GetUniLoc("s_Model");
			mLightColorLocation = mShaderProgram.GetUniLoc("s_LightColor");
			mLightPositionLocation = mShaderProgram.GetUniLoc("s_LightPosition");
			mTransInverseModelLocation = mShaderProgram.GetUniLoc("s_TransInverseModel");

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

			// Get the sampler location
			mSamplerLocation = mShaderProgram.GetUniLoc("ColorTextureSampler");
			mPositionLocation = mShaderProgram.GetAttLoc("a_Position");
			mTexCoordLocation = mShaderProgram.GetAttLoc("a_TextureCoordinate");
			mNormalLocation = mShaderProgram.GetAttLoc("a_Normal");
		}
		catch (SceneException& e)
		{
			throw SceneException(MY_FUNC, e.GetError().c_str());
		}

		SetInitialized(true);
	}

	/*
	void DiffuseModel::UpdatePosition(const SceneTime& gameTime, glm::mat4x4& mat)
	{
		static GLfloat y_angle = 0.0f;
		mat = glm::rotate(mat, y_angle, glm::vec3(0.0f, 1.0f, 0.0f));
		mat = glm::scale(mat, glm::vec3(0.003f, 0.003f, 0.003f));
		y_angle += 0.02f;
	}
	*/

	// UFO
	void DiffuseModel::UpdatePosition(const SceneTime& gameTime, glm::mat4x4& mat)
	{
		static GLfloat y_angle = 0.0f;
		static GLfloat x_angle = static_cast<GLfloat>(M_PI) / 2.0f;
		mat = glm::rotate(mat, y_angle, glm::vec3(0.0f, 1.0f, 0.0f));
		mat = glm::rotate(mat, x_angle, glm::vec3(1.0f, 0.0f, 0.0f));
		mat = glm::scale(mat, glm::vec3(0.01f, 0.01f, 0.01f));
		y_angle += 0.02f;
	}

	void DiffuseModel::Draw(const SceneTime& gameTime)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mColorTexture.getTexture());

		glUseProgram(mShaderProgram.Program());

		glm::mat4x4 mat;
		UpdatePosition(gameTime, mat);
		mat4 wvp = mCamera->ViewProjectionMatrix() * mat;
		gl::Send(mWorldViewProjectionLocation, wvp);
		glUniformMatrix4fv(mModelLocation, 1, GL_FALSE, &mat[0][0]);
		glUniform3fv(mLightColorLocation, 1, &mLightColor[0]);
		glUniform3fv(mLightPositionLocation, 1, &mLightPosition[0]);

		glm::mat4x4 transpose_inverse_model = glm::transpose(glm::inverse(mat));
		glUniformMatrix4fv(mTransInverseModelLocation, 1, GL_FALSE, &transpose_inverse_model[0][0]);

		gl::BindBuffers(mVboId, mIndexId);

		gl::AttribPtr(mPositionLocation, 4, 9, 0);
		gl::AttribPtr(mTexCoordLocation, 2, 9, 4);
		gl::AttribPtr(mNormalLocation, 3, 9, 6);

		gl::Send(mSamplerLocation, 0);

		glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, 0);

		gl::UnbindBuffers();
	}

	void DiffuseModel::CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer) const
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
			vec2 uv = (vec2) textureCoordinates[i];
			vec3 normal = normals.at(i);
			vertices.push_back(VertFormat(vec4(position.x, position.y, position.z, 1.0f), uv, normal));
		}

		CreateVertexBuffer(&vertices[0], vertices.size(), vertexBuffer);
	}

	void DiffuseModel::CreateVertexBuffer(VertFormat* vertices, GLuint vertexCount, GLuint& vertexBuffer) const
	{
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, VertexSize() * vertexCount, &vertices[0], GL_STATIC_DRAW);
	}

	size_t DiffuseModel::VertexSize() const
    {
		return sizeof(VertFormat);
    }

	void DiffuseModel::DownloadFiles()
	{
		std::string srcFolder = Utility::CombineFolder(gDownloadSingleton.getSrcFolder(), "Models");

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcImageFile,
			Library::DownloadableComponent::FileType::IMAGE_FILE_POW_OF_2);

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcObjFile,
			Library::DownloadableComponent::FileType::OBJ_FILE);

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcMtlFile,
			Library::DownloadableComponent::FileType::MTL_FILE);
	}

	bool DiffuseModel::OpenFile(const char* file, FileType file_type)
	{
		if (file_type == Library::DownloadableComponent::FileType::IMAGE_FILE_POW_OF_2)
		{
			mDestImageFile = file;
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