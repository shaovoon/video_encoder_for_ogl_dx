/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "AmbientModel.h"
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
	RTTI_DEFINITIONS(AmbientModel)

		AmbientModel::AmbientModel(Scene& game, Camera& camera, const std::string& srcImageFile, const std::string& srcObjFile, const std::string& srcMtlFile)
	: DrawableSceneComponent(game, camera), mShaderProgram(),
		mWorldViewProjectionLocation(-1), mWorldMatrix(),
		mPositionLocation(-1), mTexCoordLocation(-1), mSamplerLocation(-1),
		mVboId(-1), mIndexId(-1), mIndexCount(0), mSrcImageFile(srcImageFile),
		mSrcObjFile(srcObjFile), mSrcMtlFile(srcMtlFile)
    {
		DownloadFiles();
    }

	AmbientModel::~AmbientModel()
	{
		mColorTexture.Destroy();
		glDeleteBuffers(1, &mVboId);
		glDeleteBuffers(1, &mIndexId);
	}

	void AmbientModel::Initialize()
	{
		LOGFUNCTION("AmbientModel", "Initialize");

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
attribute vec3 a_position;

attribute vec2 a_TextureCoordinate;

varying vec2 v_TextureCoordinate;

void main()
{
    gl_Position = WorldViewProjection * vec4(a_position, 1.0);
	v_TextureCoordinate = a_TextureCoordinate;
}
)vert";

		const char* frag_shader = R"frag(
uniform sampler2D ColorTextureSampler;

varying vec2 v_TextureCoordinate;

void main()
{
    gl_FragColor = texture2D(ColorTextureSampler, v_TextureCoordinate);
//gl_FragColor = vec4(1.0);
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
			if (mColorTexture.LoadImageFile(mDestImageFile, false, false) == 0)
			{
				std::string error = "Color texture load failed.";
				error += "mDestImageFile:";
				error += mDestImageFile;
				error += ", mSrcImageFile:";
				error += mSrcImageFile;
				throw SceneException(error.c_str());
			}

			mSamplerLocation = mShaderProgram.GetUniLoc("ColorTextureSampler");
			mPositionLocation = mShaderProgram.GetAttLoc("a_position");
			mTexCoordLocation = mShaderProgram.GetAttLoc("a_TextureCoordinate");
		}
		catch (SceneException& e)
		{
			throw SceneException(MY_FUNC, e.GetError().c_str());
		}

		SetInitialized(true);
	}

	/*
	// Rose petals
	void AmbientModel::UpdatePosition(const SceneTime& gameTime, glm::mat4x4& mat)
	{
		static GLfloat y_angle = 0.0f;
		mat = glm::rotate(mat, y_angle, glm::vec3(0.0f, 1.0f, 0.0f));
		mat = glm::scale(mat, glm::vec3(0.003f, 0.003f, 0.003f));
		y_angle += 0.02f;
	}
	*/

	/*
	// maple leaves
	void AmbientModel::UpdatePosition(const SceneTime& gameTime, glm::mat4x4& mat)
	{
		static GLfloat y_angle = 0.0f;
		static GLfloat x_angle = static_cast<GLfloat>(M_PI) / 2.0f;
		mat = glm::rotate(mat, y_angle, glm::vec3(0.0f, 1.0f, 0.0f));
		mat = glm::rotate(mat, x_angle, glm::vec3(1.0f, 0.0f, 0.0f));
		mat = glm::scale(mat, glm::vec3(0.1f, 0.1f, 0.1f));
		y_angle += 0.02f;
	}
	*/

	// UFO
	void AmbientModel::UpdatePosition(const SceneTime& gameTime, glm::mat4x4& mat)
	{
		static GLfloat y_angle = 0.0f;
		static GLfloat x_angle = static_cast<GLfloat>(M_PI) / 2.0f;
		mat = glm::rotate(mat, y_angle, glm::vec3(0.0f, 1.0f, 0.0f));
		mat = glm::rotate(mat, x_angle, glm::vec3(1.0f, 0.0f, 0.0f));
		mat = glm::scale(mat, glm::vec3(0.01f, 0.01f, 0.01f));
		y_angle += 0.02f;
	}

	/*
	// dark grey UFO
	void AmbientModel::UpdatePosition(const SceneTime& gameTime, glm::mat4x4& mat)
	{
		static GLfloat y_angle = 0.0f;
		static GLfloat x_angle = static_cast<GLfloat>(M_PI) / 2.0f;
		glm::mat4x4 s_mat = glm::scale(glm::vec3(0.1f, 0.1f, 0.1f));
		glm::mat4x4 r_mat_x = glm::rotate(x_angle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4x4 r_mat_y = glm::rotate(y_angle, glm::vec3(0.0f, 1.0f, 0.0f));
		y_angle += 0.02f;

		mat = r_mat_y * r_mat_x * s_mat;
	}
	*/
	void AmbientModel::Draw(const SceneTime& gameTime)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mColorTexture.getTexture());

		glUseProgram(mShaderProgram.Program());

		glm::mat4x4 mat;
		UpdatePosition(gameTime, mat);
		mat4 wvp = mCamera->ViewProjectionMatrix() * mat;
		gl::Send(mWorldViewProjectionLocation, wvp);

		gl::BindBuffers(mVboId, mIndexId);

		gl::AttribPtr(mPositionLocation, 4, 6, 0);
		gl::AttribPtr(mTexCoordLocation, 2, 6, 4);

		gl::Send(mSamplerLocation, 0);

		// Draw 
		glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, 0);

		gl::UnbindBuffers();
	}

	void AmbientModel::CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer) const
	{
		const std::vector<vec3>& sourceVertices = mesh.Vertices();

		std::vector<VertFormat> vertices;
		vertices.reserve(sourceVertices.size());

		const std::vector<glm::vec2>& textureCoordinates = mesh.TextureCoordinates();
		assert(textureCoordinates.size() == sourceVertices.size());

		const GLfloat MAX = 10000.0f;
		GLfloat minX =  MAX;
		GLfloat maxX = -MAX;
		GLfloat minY =  MAX;
		GLfloat maxY = -MAX;
		GLfloat minZ =  MAX;
		GLfloat maxZ = -MAX;

		for (size_t i = 0; i < sourceVertices.size(); i++)
		{
			vec3 position = sourceVertices.at(i);
			vec2 uv = (vec2) textureCoordinates[i];
			vertices.push_back(VertFormat(vec4(position.x, position.y, position.z, 1.0f), uv));

			if (position.x > maxX)
				maxX = position.x;
			if (position.y > maxY)
				maxY = position.y;
			if (position.z > maxZ)
				maxZ = position.z;
			
			if (position.x < minX)
				minX = position.x;
			if (position.y < minY)
				minY = position.y;
			if (position.z < minZ)
				minZ = position.z;
		}

		char buf[1000];
		SPRINTF(buf, "minX:%f, maxX:%f, minY:%f, maxY:%f, minZ:%f, maxZ:%f\n", minX, maxX, minY, maxY, minZ, maxZ);

		Logger::DebugPrint(buf);

		CreateVertexBuffer(&vertices[0], vertices.size(), vertexBuffer);
	}

	void AmbientModel::CreateVertexBuffer(VertFormat* vertices, GLuint vertexCount, GLuint& vertexBuffer) const
	{
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, VertexSize() * vertexCount, &vertices[0], GL_STATIC_DRAW);
	}

	size_t AmbientModel::VertexSize() const
    {
		return sizeof(VertFormat);
    }

	void AmbientModel::DownloadFiles()
	{
		std::string srcFolder = Utility::CombineFolder(gDownloadSingleton.getSrcFolder(), "Models");

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcImageFile,
			Library::DownloadableComponent::FileType::IMAGE_FILE_POW_OF_2);

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcObjFile,
			Library::DownloadableComponent::FileType::OBJ_FILE);

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcMtlFile,
			Library::DownloadableComponent::FileType::MTL_FILE);
	}

	bool AmbientModel::OpenFile(const char* file, FileType file_type)
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