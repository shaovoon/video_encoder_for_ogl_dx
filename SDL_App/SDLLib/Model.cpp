/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "Model.h"
#include "Scene.h"
#include "SceneException.h"
#include "Mesh.h"
#include "ModelMaterial.h"
#include "Utility.h"

namespace Library
{
    Model::Model(Scene& game, const std::string& filename, bool flipUVs)
		: mGame(game), mMeshes(), mMaterials()
    {
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;

		std::string directory = "";
#ifndef __EMSCRIPTEN__
		Utility::GetDirectory(filename, directory);
		directory += "\\";
#endif
		if (tinyobj::LoadObj(shapes, materials, err, filename.c_str(), directory.c_str()))
		{
			for (size_t i = 0; i < shapes.size(); ++i)
			{
				Mesh* mesh = new Mesh(*this, shapes[i]);
				mMeshes.push_back(mesh);
			}
			for (size_t i = 0; i < materials.size(); ++i)
			{
				mMTL.push_back(materials[i]);
			}
		}
	}
	
    Model::~Model()
    {
        for (Mesh* mesh : mMeshes)
        {
            delete mesh;
        }

        for (ModelMaterial* material : mMaterials)
        {
            delete material;
        }
    }

    Scene& Model::GetGame()
    {
        return mGame;
    }

    bool Model::HasMeshes() const
    {
        return (mMeshes.size() > 0);
    }

    bool Model::HasMaterials() const
    {
        return (mMaterials.size() > 0);
    }

    const std::vector<Mesh*>& Model::Meshes() const
    {
        return mMeshes;
    }

    const std::vector<ModelMaterial*>& Model::Materials() const
    {
        return mMaterials;
    }
	const std::vector<tinyobj::material_t>& Model::MTLs() const
	{
		return mMTL;
	}

	glm::vec4 Model::GetAmbientColor(size_t index) const
	{
		if (index >= mMTL.size())
		{
			throw SceneException(MY_FUNC, "No Ambient Color");
		}

		const tinyobj::material_t& mat = mMTL[index];
		glm::vec4 color(mat.ambient[0], mat.ambient[1], mat.ambient[2], 1.0f);
		return color;
	}
	glm::vec4 Model::GetDiffuseColor(size_t index) const
	{
		if (index >= mMTL.size())
		{
			throw SceneException(MY_FUNC, "No Diffuse Color");
		}

		const tinyobj::material_t& mat = mMTL[index];
		glm::vec4 color(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1.0f);
		return color;
	}
	glm::vec4 Model::GetSpecularColor(size_t index) const
	{
		if (index >= mMTL.size())
		{
			throw SceneException(MY_FUNC, "No Specular Color");
		}

		const tinyobj::material_t& mat = mMTL[index];
		glm::vec4 color(mat.specular[0], mat.specular[1], mat.specular[2], 1.0f);
		return color;
	}
	GLfloat Model::GetShininess(size_t index) const
	{
		if (index >= mMTL.size())
		{
			throw SceneException(MY_FUNC, "No Shininess Color");
		}

		const tinyobj::material_t& mat = mMTL[index];
		return mat.shininess;
	}

}
