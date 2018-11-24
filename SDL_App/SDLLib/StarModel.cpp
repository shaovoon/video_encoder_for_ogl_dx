/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#include "StarModel.h"
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
	RTTI_DEFINITIONS(StarModel)

	StarModel::StarModel(Scene& game, Camera& camera, const glm::vec3& color, const std::string& srcObjFile, const std::string& srcMtlFile)
	: DrawableSceneComponent(game, camera), mShaderProgram(),
		mWorldViewProjectionLocation(-1), mModelLocation(-1), mWorldMatrix(),
		mPositionLocation(-1), mNormalLocation(-1),
		mVboId(-1), mIndexId(-1), mIndexCount(0), mModelColor(color), mLightColor(1.0f, 1.0f, 1.0f), mLightPosition(1.0f,0.4f,0.7f),
		mSrcObjFile(srcObjFile), mSrcMtlFile(srcMtlFile), mModelColorLocation(-1), mLightColorLocation(-1), 
		mLightPositionLocation(-1), mTransInverseModelLocation(-1), mViewPosLocation(-1)
    {
		DownloadFiles();
    }

	StarModel::~StarModel()
	{
		glDeleteBuffers(1, &mVboId);
	}

	void StarModel::Initialize()
	{
		LOGFUNCTION("StarModel", "Initialize");

		// Load the model
		std::unique_ptr<Model> model(new Model(*mGame, mDestObjFile, true));

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
attribute vec3 a_Normal;

varying vec3 v_Normal;
varying vec3 v_FragPos;

void main()
{
    gl_Position = WorldViewProjection * vec4(a_Position, 1.0);
	v_FragPos = vec3(s_Model * vec4(a_Position, 1.0));
	v_Normal = mat3(s_TransInverseModel) * a_Normal; 
}
)vert";

		const char* frag_shader = R"frag(
uniform vec3 s_ModelColor;
uniform vec3 s_LightColor;
uniform vec3 s_LightPosition;
uniform vec3 s_ViewPos;

varying vec3 v_Normal;
varying vec3 v_FragPos;

void main()
{
	vec3 norm = normalize(v_Normal);
	vec3 lightDir = normalize(s_LightPosition - v_FragPos);  

	float specularStrength = 0.5;
	vec3 viewDir = normalize(s_ViewPos - v_FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);  

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
	vec3 specular = specularStrength * spec * s_LightColor; 

    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * s_LightColor;

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * s_LightColor;

	vec3 result = (ambient + diffuse + specular) * s_ModelColor.rgb;
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
			mModelColorLocation = mShaderProgram.GetUniLoc("s_ModelColor");
			mLightColorLocation = mShaderProgram.GetUniLoc("s_LightColor");
			mLightPositionLocation = mShaderProgram.GetUniLoc("s_LightPosition");
			mTransInverseModelLocation = mShaderProgram.GetUniLoc("s_TransInverseModel");
			mViewPosLocation = mShaderProgram.GetUniLoc("s_ViewPos");
			mPositionLocation = mShaderProgram.GetAttLoc("a_Position");
			mNormalLocation = mShaderProgram.GetAttLoc("a_Normal");
		}
		catch (SceneException& e)
		{
			throw SceneException(MY_FUNC, e.GetError().c_str());
		}

		SetInitialized(true);
	}

	void StarModel::UpdatePosition(const SceneTime& gameTime, glm::mat4x4& mat)
	{
		static GLfloat y_angle = 0.0f;
		static GLfloat x_angle = static_cast<GLfloat>(M_PI) / 2.0f;
		mat = glm::rotate(mat, y_angle, glm::vec3(0.0f, 1.0f, 0.0f));
		//mat = glm::rotate(mat, x_angle, glm::vec3(1.0f, 0.0f, 0.0f));
		mat = glm::scale(mat, glm::vec3(0.3f, 0.3f, 0.3f));
		y_angle += 0.02f;
	}

	void StarModel::Draw(const SceneTime& gameTime)
	{
		glUseProgram(mShaderProgram.Program());

		glm::mat4x4 mat;
		UpdatePosition(gameTime, mat);
		mat4 wvp = mCamera->ViewProjectionMatrix() * mat;
		gl::Send(mWorldViewProjectionLocation, wvp);
		glUniformMatrix4fv(mModelLocation, 1, GL_FALSE, &mat[0][0]);
		glUniform3fv(mModelColorLocation, 1, &mModelColor[0]);
		glUniform3fv(mLightColorLocation, 1, &mLightColor[0]);
		glUniform3fv(mLightPositionLocation, 1, &mLightPosition[0]);
		glUniform3fv(mViewPosLocation, 1, &mCamera->Position()[0]);

		glm::mat4x4 transpose_inverse_model = glm::transpose(glm::inverse(mat));
		glUniformMatrix4fv(mTransInverseModelLocation, 1, GL_FALSE, &transpose_inverse_model[0][0]);

		gl::BindBuffers(mVboId, mIndexId);

		gl::AttribPtr(mPositionLocation, 4, 7, 0);
		gl::AttribPtr(mNormalLocation, 3, 7, 4);

		glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void StarModel::CreateVertexBuffer(const Mesh& mesh, GLuint& vertexBuffer) const
	{
		const std::vector<vec3>& sourceVertices = mesh.Vertices();

		std::vector<VertFormat> vertices;
		vertices.reserve(sourceVertices.size());

		const std::vector<vec3>& normals = mesh.Normals();
		assert(normals.size() == sourceVertices.size());

		for (size_t i = 0; i < sourceVertices.size(); i++)
		{
			vec3 position = sourceVertices.at(i);
			vec3 normal = normals.at(i);
			vertices.push_back(VertFormat(vec4(position.x, position.y, position.z, 1.0f), normal));
		}

		CreateVertexBuffer(&vertices[0], vertices.size(), vertexBuffer);
	}

	void StarModel::CreateVertexBuffer(VertFormat* vertices, GLuint vertexCount, GLuint& vertexBuffer) const
	{
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, VertexSize() * vertexCount, &vertices[0], GL_STATIC_DRAW);
	}

	size_t StarModel::VertexSize() const
    {
		return sizeof(VertFormat);
    }

	void StarModel::DownloadFiles()
	{
		std::string srcFolder = Utility::CombineFolder(gDownloadSingleton.getSrcFolder(), "Models");

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcObjFile,
			Library::DownloadableComponent::FileType::OBJ_FILE);

		gDownloadSingleton.DownloadFile(this, srcFolder, mSrcMtlFile,
			Library::DownloadableComponent::FileType::MTL_FILE);
	}

	bool StarModel::OpenFile(const char* file, FileType file_type)
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