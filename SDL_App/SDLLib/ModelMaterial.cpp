/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "ModelMaterial.h"
#include "SceneException.h"
#include "Utility.h"

namespace Library
{
	std::map<TextureType, unsigned int> ModelMaterial::sTextureTypeMappings;

	ModelMaterial::ModelMaterial(Model& model)
        : mModel(model), mTextures()
    {
        InitializeTextureTypeMappings();
    }

    ModelMaterial::~ModelMaterial()
    {
        for (std::pair<TextureType, std::vector<std::wstring>*> textures : mTextures)
        {
			if (textures.second)
			{
				delete textures.second;
				textures.second = nullptr;
			}
        }
    }

    Model& ModelMaterial::GetModel()
    {
        return mModel;
    }

    const std::string& ModelMaterial::Name() const
    {
        return mName;
    }

    const std::map<TextureType, std::vector<std::wstring>*>& ModelMaterial::Textures() const
    {
        return mTextures;
    }

    void ModelMaterial::InitializeTextureTypeMappings()
    {
    }
}