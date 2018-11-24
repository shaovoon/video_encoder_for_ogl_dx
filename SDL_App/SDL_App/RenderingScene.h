#pragma once

#include "Scene.h"
#include "Texture.h"
#include "FirstPersonCamera.h"
#include "TexturedDemo.h"
#include "DiffuseModel.h"
#include "DiffuseCube.h"
#include "SpecularModel.h"
#include "StarModel.h"
#include "Enums.h"

using namespace Library;

namespace Library
{
	class SceneTime;
}

namespace Rendering
{
	class RenderingScene : public Scene
	{
		RTTI_DECLARATIONS(RenderingScene, Scene)

	public:
		RenderingScene(const std::wstring& windowTitle, unsigned int screenWidth, unsigned int screenHeight);

		virtual bool IsAllReady() override;
		virtual void CreateComponents() override;
		virtual bool IsAllInitialized() override;

	protected:
		virtual void Initialize() override;
		virtual void Draw(const SceneTime& gameTime) override;
		virtual void Shutdown() override;

	private:
		GLfloat mFloatScreenWidth;
		GLfloat mFloatScreenHeight;
		std::unique_ptr<FirstPersonCamera> mCamera;
		std::unique_ptr<TexturedDemo> mTexturedModelDemo;
		std::unique_ptr<DiffuseCube> mDiffuseCube;
		std::unique_ptr<SpecularModel> mUFOSpecularModel;
		std::unique_ptr<StarModel> mStarModel;
	};
}
