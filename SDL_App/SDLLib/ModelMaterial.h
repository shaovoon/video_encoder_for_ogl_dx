/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#pragma once

#include "Common.h"
#include "Model.h"

namespace Library
{
    enum TextureType
    {
        TextureTypeDifffuse = 0,
        TextureTypeSpecularMap,
        TextureTypeAmbient,
        TextureTypeEmissive,
        TextureTypeHeightmap,
        TextureTypeNormalMap,
        TextureTypeSpecularPowerMap,
        TextureTypeDisplacementMap,
        TextureTypeLightMap,
        TextureTypeEnd
    };

    class ModelMaterial
    {
        friend class Model;

    public:
		ModelMaterial(Model& model);
        ~ModelMaterial();

        Model& GetModel();
        const std::string& Name() const;
        const std::map<TextureType, std::vector<std::wstring>*>& Textures() const;

    private:		
        static void InitializeTextureTypeMappings();
		static std::map<TextureType, unsigned int> sTextureTypeMappings;

        ModelMaterial(const ModelMaterial& rhs);
        ModelMaterial& operator=(const ModelMaterial& rhs);

        Model& mModel;
        std::string mName;
        std::map<TextureType, std::vector<std::wstring>*> mTextures;		
    };
}