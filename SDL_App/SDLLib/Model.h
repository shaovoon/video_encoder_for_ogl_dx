/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#pragma once

#include "Common.h"
#include "tiny_obj_loader.h"

namespace Library
{
    class Scene;
    class Mesh;
    class ModelMaterial;
	class AnimationClip;

    class Model
    {
		friend class Mesh;

    public:
        Model(Scene& game, const std::string& filename, bool flipUVs = false);
        ~Model();

        Scene& GetGame();
        bool HasMeshes() const;
        bool HasMaterials() const;

        const std::vector<Mesh*>& Meshes() const;
        const std::vector<ModelMaterial*>& Materials() const;
		const std::vector<tinyobj::material_t>& MTLs() const;

		glm::vec4 GetAmbientColor(size_t index) const;
		glm::vec4 GetDiffuseColor(size_t index) const;
		glm::vec4 GetSpecularColor(size_t index) const;

		GLfloat GetShininess(size_t index) const;

    private:
        Model(const Model& rhs);
        Model& operator=(const Model& rhs);

        Scene& mGame;
        std::vector<Mesh*> mMeshes;
        std::vector<ModelMaterial*> mMaterials;
		std::vector<tinyobj::material_t> mMTL;
    };
}
