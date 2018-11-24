/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "BinaryModelList.h"
#include "SceneException.h"
#include "Mesh.h"
#include "ColorHelper.h"
#include "VertexBufferHelper.h"
#include "Utility.h"
#include "Camera.h"
#include "DownloadSingleton.h"
#include "VariaNetDebugPrint.h"
#include "FileJoiner.h"
#include "Decrypt.h"

extern Library::DownloadSingleton gDownloadSingleton;

using namespace glm;

namespace Library
{
	RTTI_DEFINITIONS(BinaryModelList)

		BinaryModelList::BinaryModelList(Scene& game, Camera& camera, const std::string& srcImageFile, const std::string& srcAllFiles, 
			const std::vector<std::string>& srcObjFiles, const std::vector<size_t>& vecFileSize, const std::string& srcMtlFile, Precision precision)
	: DrawableSceneComponent(game, camera), mShaderProgram(),
		mWorldViewProjectionLocation(-1), mModelLocation(-1), mWorldMatrix(),
		mPositionLocation(-1), mNormalLocation(-1),
		mLightColor(1.0f, 1.0f, 1.0f),	mLightPosition(1.0f, 0.2f, 1.0f),
		mSrcImageFile(srcImageFile), mSrcAllFiles(srcAllFiles), 
		mSrcObjFiles(srcObjFiles), mVecFileSize(vecFileSize),
		mSrcMtlFile(srcMtlFile), mLightColorLocation(-1), 
		mLightPositionLocation(-1), mTransInverseModelLocation(-1),
		mPrecision(precision)
    {
		DownloadFiles();
    }

	BinaryModelList::~BinaryModelList()
	{
		mColorTexture.Destroy();
		for (size_t i = 0; i < mModelVertices.size(); ++i)
		{
			ModelVertices& m = mModelVertices[i];
			glDeleteBuffers(1, &m.mVboId);
			glDeleteBuffers(1, &m.mIndexId);
			m.mIndexCount = 0;
		}
	}

	void BinaryModelList::LoadModel(const std::string& file)
	{
		ModelVertices m(file);
		m.mDestObjFile = file;
		// Load the model
		std::unique_ptr<Model> model(new Model(*mGame, m.mDestObjFile, true));

		// Create the vertex and index buffers
		Mesh* mesh = model->Meshes().at(0);
		CreateVertexBuffer(*mesh, m.mVboId);
		mesh->CreateIndexBuffer(m.mIndexId);
		m.mIndexCount = mesh->Indices().size();
		mModelVertices.push_back(m);
	}
	void BinaryModelList::Initialize()
	{
		LOGFUNCTION("BinaryModelList", "Initialize");

#ifdef _MSC_VER
		std::string srcFolder = Utility::CombineFolder(gDownloadSingleton.getSrcFolder(), "Models");
		for (size_t i = 0; i < mSrcObjFiles.size(); ++i)
		{
			mSrcObjFiles[i] = Utility::CombineFilePath(srcFolder, mSrcObjFiles[i]);
			LoadModel(mSrcObjFiles[i]);
		}
#else
		std::string obj_file = mDestAllFiles;
		if (mDestAllFiles.find(".gz") == mDestAllFiles.size() - 3) // gz compressed file
		{
			obj_file = obj_file.substr(0, obj_file.size() - 3);
		}
		if (mDestAllFiles.find(".zip") == mDestAllFiles.size() - 4) // gz compressed file, ignore the zip ext.
		{
			obj_file = obj_file.substr(0, obj_file.size() - 4);
		}

		Utility::DecompressFile(mDestAllFiles.c_str(), obj_file.c_str());
		if (obj_file.find(".bin") == obj_file.size() - 4) // gz compressed file, ignore the zip ext.
		{
			const std::string bin_file = obj_file;
			obj_file = obj_file.substr(0, obj_file.size() - 4);
			obj_file += ".txt";
			Decryption::decrypt_file(bin_file, obj_file);
		}
		if (FileJoiner::scatter(mSrcObjFiles, mVecFileSize, obj_file))
		{
			for (size_t i = 0; i < mSrcObjFiles.size(); ++i)
			{
				LoadModel(mSrcObjFiles[i]);
			}
		}
#endif
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

	vec4 sampledColor = texture2D(ColorTextureSampler, v_TextureCoordinate);

	if(diff > 0.8)
	{
		gl_FragColor = sampledColor;
	}
	else 
	{
		vec3 diffuse = diff * s_LightColor;
		vec3 result = (ambient + diffuse) * sampledColor.rgb;
		gl_FragColor = vec4(result, 1.0);
	}
}
)frag";

		// Build the shader program
		std::vector<ShaderSourceDefinition> shaders;
		shaders.push_back(ShaderSourceDefinition(GL_VERTEX_SHADER, vert_shader));
		shaders.push_back(ShaderSourceDefinition(GL_FRAGMENT_SHADER, frag_shader));
		try
		{
			mShaderProgram.BuildProgramFromSource(shaders, mPrecision);

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

	void BinaryModelList::UpdatePosition(const SceneTime& gameTime, glm::mat4x4& mat)
	{
		static GLfloat y_angle = 0.0f;
		static GLfloat x_angle = static_cast<GLfloat>(M_PI) / 2.0f;
		glm::mat4x4 s_mat = glm::scale(glm::vec3(0.1f, 0.1f, 0.1f));
		glm::mat4x4 r_mat_x = glm::rotate(x_angle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4x4 r_mat_y = glm::rotate(y_angle, glm::vec3(0.0f, 1.0f, 0.0f));
		y_angle += 0.02f;

		mat = r_mat_y * r_mat_x * s_mat;
	}

	void BinaryModelList::Draw(const SceneTime& gameTime, size_t index, glm::mat4x4& mat)
	{
		if (index >= mModelVertices.size())
		{
			return;
		}

		ModelVertices& m = mModelVertices[index];

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mColorTexture.getTexture());

		glUseProgram(mShaderProgram.Program());

		mat4 wvp = mCamera->ViewProjectionMatrix() * mat;
		gl::Send(mWorldViewProjectionLocation, wvp);
		glUniformMatrix4fv(mModelLocation, 1, GL_FALSE, &mat[0][0]);
		glUniform3fv(mLightColorLocation, 1, &mLightColor[0]);
		glUniform3fv(mLightPositionLocation, 1, &mLightPosition[0]);

		glm::mat4x4 transpose_inverse_model = glm::transpose(glm::inverse(mat));
		glUniformMatrix4fv(mTransInverseModelLocation, 1, GL_FALSE, &transpose_inverse_model[0][0]);

		gl::BindBuffers(m.mVboId, m.mIndexId);

		gl::AttribPtr(mPositionLocation, 4, 9, 0);
		gl::AttribPtr(mTexCoordLocation, 2, 9, 4);
		gl::AttribPtr(mNormalLocation, 3, 9, 6);

		gl::Send(mSamplerLocation, 0);

		glDrawElements(GL_TRIANGLES, m.mIndexCount, GL_UNSIGNED_INT, 0);

		gl::UnbindBuffers();
	}

	void BinaryModelList::DrawPrologue()
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mColorTexture.getTexture());

		glUseProgram(mShaderProgram.Program());

		glUniform3fv(mLightColorLocation, 1, &mLightColor[0]);
		glUniform3fv(mLightPositionLocation, 1, &mLightPosition[0]);
		gl::Send(mSamplerLocation, 0);
	}
	void BinaryModelList::DrawVertices(const SceneTime& gameTime, size_t index, glm::mat4x4& mat)
	{
		if (index >= mModelVertices.size())
		{
			return;
		}

		ModelVertices& m = mModelVertices[index];

		mat4 wvp = mCamera->ViewProjectionMatrix() * mat;
		gl::Send(mWorldViewProjectionLocation, wvp);
		glUniformMatrix4fv(mModelLocation, 1, GL_FALSE, &mat[0][0]);

		glm::mat4x4 transpose_inverse_model = glm::transpose(glm::inverse(mat));
		glUniformMatrix4fv(mTransInverseModelLocation, 1, GL_FALSE, &transpose_inverse_model[0][0]);

		gl::BindBuffers(m.mVboId, m.mIndexId);

		gl::AttribPtr(mPositionLocation, 4, 9, 0);
		gl::AttribPtr(mTexCoordLocation, 2, 9, 4);
		gl::AttribPtr(mNormalLocation, 3, 9, 6);

		glDrawElements(GL_TRIANGLES, m.mIndexCount, GL_UNSIGNED_INT, 0);
	}
	void BinaryModelList::DrawEpilogue()
	{
		gl::UnbindBuffers();
	}


	void BinaryModelList::Draw(const SceneTime& gameTime)
	{
		glm::mat4x4 mat;
		Draw(gameTime, 0, mat);
	}

	void BinaryModelList::CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer) const
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

	void BinaryModelList::CreateVertexBuffer(VertFormat* vertices, GLuint vertexCount, GLuint& vertexBuffer) const
	{
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, VertexSize() * vertexCount, &vertices[0], GL_STATIC_DRAW);
	}

	size_t BinaryModelList::VertexSize() const
    {
		return sizeof(VertFormat);
    }

	void BinaryModelList::DownloadFiles()
	{
		std::string srcFolder = Utility::CombineFolder(gDownloadSingleton.getSrcFolder(), "Models");

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcImageFile,
			Library::DownloadableComponent::FileType::IMAGE_FILE_POW_OF_2);

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcAllFiles,
			Library::DownloadableComponent::FileType::ALL_OBJ_GZ_FILE);

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcMtlFile,
			Library::DownloadableComponent::FileType::MTL_FILE);
	}

	bool BinaryModelList::OpenFile(const char* file, FileType file_type)
	{
		if (file_type == Library::DownloadableComponent::FileType::IMAGE_FILE_POW_OF_2)
		{
			mDestImageFile = file;
			return true;
		}
		if (file_type == Library::DownloadableComponent::FileType::ALL_OBJ_GZ_FILE)
		{
			mDestAllFiles = file;
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