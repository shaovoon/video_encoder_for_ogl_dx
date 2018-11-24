/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#pragma once

#include "Common.h"
#include "Model.h"
#include "tiny_obj_loader.h"

namespace Library
{
    class Material;
    class ModelMaterial;

    class Mesh
    {
        friend class Model;

    public:
		~Mesh();

        Model& GetModel();
        ModelMaterial* GetMaterial();
        const std::string& Name() const;

        const std::vector<glm::vec3>& Vertices() const;
        const std::vector<glm::vec3>& Normals() const;
        const std::vector<glm::vec3>& Tangents() const;
        const std::vector<glm::vec3>& BiNormals() const;
        const std::vector<glm::vec2>& TextureCoordinates() const;
        const std::vector<glm::vec4>& VertexColors() const;
        unsigned int FaceCount() const;
		const std::vector<unsigned int>& Indices() const;

        void CreateIndexBuffer(GLuint& indexBuffer);

    private:
		Mesh(Model& model, tinyobj::shape_t& mesh);
        Mesh(const Mesh& rhs);
        Mesh& operator=(const Mesh& rhs);

        Model& mModel;
        ModelMaterial* mMaterial;
        std::string mName;
        std::vector<glm::vec3> mVertices;
        std::vector<glm::vec3> mNormals;
        std::vector<glm::vec3> mTangents;
        std::vector<glm::vec3> mBiNormals;
        std::vector<glm::vec2> mTextureCoordinates;
        std::vector<glm::vec4> mVertexColors;
		unsigned int mFaceCount;
		std::vector<unsigned int> mIndices;
    };
}