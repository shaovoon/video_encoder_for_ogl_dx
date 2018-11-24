/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "Cube.h"
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
	RTTI_DEFINITIONS(Cube)

		Cube::Cube(Scene& game, Camera& camera, const std::string& srcObjFile, const std::string& srcMtlFile)
		: DrawableSceneComponent(game, camera), mShaderProgram(),
		mWorldViewProjectionLocation(-1), mWorldMatrix(),
		mPositionLocation(-1),
		mVboId(-1), mIndexId(-1), mIndexCount(0),
		mSrcObjFile(srcObjFile), mSrcMtlFile(srcMtlFile)
	{
		DownloadFiles();
	}

	Cube::~Cube()
	{
		glDeleteBuffers(1, &mVboId);
		glDeleteBuffers(1, &mIndexId);
	}

	void Cube::Initialize()
	{
		LOGFUNCTION("Cube", "Initialize");

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
void main()
{
    gl_Position = WorldViewProjection * vec4(a_position, 1.0);
}
)vert";

		const char* frag_shader = R"frag(
void main()
{
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
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
		}
		catch (SceneException& e)
		{
			throw SceneException(MY_FUNC, e.GetError().c_str());
		}

		SetInitialized(true);
	}

	void Cube::UpdatePosition(const SceneTime& gameTime, glm::mat4x4& mat)
	{
		static GLfloat y_angle = 0.0f;
		mat = glm::rotate(mat, y_angle, glm::vec3(0.0f, 1.0f, 0.0f));
		mat = glm::rotate(mat, 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));
		mat = glm::scale(mat, glm::vec3(0.1f, 0.1f, 0.1f));
		y_angle += 0.02f;
	}

	void Cube::Draw(const SceneTime& gameTime)
	{
		glUseProgram(mShaderProgram.Program());

		glm::mat4x4 mat;
		UpdatePosition(gameTime, mat);
		mat4 wvp = mCamera->ViewProjectionMatrix() * mat;
		gl::Send(mWorldViewProjectionLocation, wvp);

		gl::BindBuffers(mVboId, mIndexId);

		gl::AttribPtr(mPositionLocation, 4, 4, 0);

		glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, 0);

		gl::UnbindBuffers();
	}

	void Cube::CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer) const
	{
		const std::vector<vec3>& sourceVertices = mesh.Vertices();

		std::vector<VertFormat> vertices;
		vertices.reserve(sourceVertices.size());

		for (size_t i = 0; i < sourceVertices.size(); i++)
		{
			vec3 position = sourceVertices.at(i);
			vertices.push_back(VertFormat(vec4(position.x, position.y, position.z, 1.0f)));
		}

		CreateVertexBuffer(&vertices[0], vertices.size(), vertexBuffer);
	}

	void Cube::CreateVertexBuffer(VertFormat* vertices, GLuint vertexCount, GLuint& vertexBuffer) const
	{
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, VertexSize() * vertexCount, &vertices[0], GL_STATIC_DRAW);
	}

	size_t Cube::VertexSize() const
    {
		return sizeof(VertFormat);
    }

	void Cube::DownloadFiles()
	{
		std::string srcFolder = Utility::CombineFolder(gDownloadSingleton.getSrcFolder(), "Models");

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcObjFile,
			Library::DownloadableComponent::FileType::OBJ_FILE);

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcMtlFile,
			Library::DownloadableComponent::FileType::MTL_FILE);
	}

	bool Cube::OpenFile(const char* file, FileType file_type)
	{
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