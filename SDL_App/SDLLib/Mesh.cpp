/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "Mesh.h"
#include "Model.h"
#include "Scene.h"
#include "SceneException.h"

namespace Library
{
	Mesh::Mesh(Model& model, tinyobj::shape_t& shape)
		: mModel(model), mMaterial(nullptr), mName(shape.name), mVertices(), mNormals(), mTangents(), mBiNormals(), mTextureCoordinates(), mVertexColors(),
		  mFaceCount(0), mIndices()
    {
		tinyobj::mesh_t& mesh = shape.mesh;
		mVertices.reserve(mesh.positions.size()/3);
		for (size_t i = 0; i < mesh.positions.size(); i += 3)
		{
			mVertices.push_back(glm::vec3(mesh.positions[i], mesh.positions[i + 1], mesh.positions[i + 2]));
		}

		if (mesh.normals.size()>0)
		{
			mNormals.reserve(mesh.normals.size()/3);
			for (size_t i = 0; i < mesh.normals.size(); i += 3)
			{
				mNormals.push_back(glm::vec3(mesh.normals[i], mesh.normals[i + 1], mesh.normals[i + 2]));
			}
		}

		if (mesh.texcoords.size()>0)
		{
			mTextureCoordinates.reserve(mesh.texcoords.size() / 2);
			for (size_t i = 0; i < mesh.texcoords.size(); i += 2)
			{
				mTextureCoordinates.push_back(glm::vec2(mesh.texcoords[i], mesh.texcoords[i + 1]));
			}
		}

		if (mesh.indices.size())
		{
			mFaceCount = 1;

			for (size_t j = 0; j < mesh.indices.size(); j++)
			{
				mIndices.push_back(mesh.indices[j]);
			}
		}
    }

    Mesh::~Mesh()
    {
    }

    Model& Mesh::GetModel()
    {
        return mModel;
    }

    ModelMaterial* Mesh::GetMaterial()
    {
        return mMaterial;
    }

    const std::string& Mesh::Name() const
    {
        return mName;
    }

    const std::vector<glm::vec3>& Mesh::Vertices() const
    {
        return mVertices;
    }

    const std::vector<glm::vec3>& Mesh::Normals() const
    {
        return mNormals;
    }

    const std::vector<glm::vec3>& Mesh::Tangents() const
    {
        return mTangents;
    }

    const std::vector<glm::vec3>& Mesh::BiNormals() const
    {
        return mBiNormals;
    }

    const std::vector<glm::vec2>& Mesh::TextureCoordinates() const
    {
        return mTextureCoordinates;
    }

    const std::vector<glm::vec4>& Mesh::VertexColors() const
    {
        return mVertexColors;
    }

	unsigned int Mesh::FaceCount() const
    {
        return mFaceCount;
    }

	const std::vector<unsigned int>& Mesh::Indices() const
    {
        return mIndices;
    }

    void Mesh::CreateIndexBuffer(GLuint& indexBuffer)
    {
		glGenBuffers(1, &indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mIndices.size(), &mIndices[0], GL_STATIC_DRAW);
    }
}