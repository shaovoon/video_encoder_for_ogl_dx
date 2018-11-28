#include "RenderingScene.h"
#include "SceneException.h"
#include "ColorHelper.h"
#include "VectorHelper.h"
#include "ConfigSingleton.h"
#include <DownloadSingleton.h>
#include <VariaNetDebugPrint.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

extern ConfigSingleton gConfigSingleton;
extern DownloadSingleton gDownloadSingleton;
extern Logger gLogger;

using namespace glm;

namespace Rendering
{
	RTTI_DEFINITIONS(RenderingScene)

	RenderingScene::RenderingScene(const std::wstring& windowTitle, unsigned int screenWidth, unsigned int screenHeight)
		: Scene(windowTitle, screenWidth, screenHeight),
		mFloatScreenWidth(2.0f),
		mFloatScreenHeight(1.5f)
	{
	}

	bool RenderingScene::IsAllReady()
	{
		bool ready = true;
		for (SceneComponent* comp : mComponents)
		{
			DownloadableComponent* downloadable = comp->As<DownloadableComponent>();
			ready &= downloadable->DownloadReady();
		}
		return ready;
	}
	bool RenderingScene::IsAllInitialized()
	{
		bool initialized = true;
		for (SceneComponent* comp : mComponents)
		{
			DownloadableComponent* downloadable = comp->As<DownloadableComponent>();
			initialized &= downloadable->IsInitialized();
		}
		return initialized;
	}

	void RenderingScene::CreateComponents()
	{
		mCamera = std::unique_ptr<FirstPersonCamera>(new FirstPersonCamera(*this, 45.0f, 1.0f / 0.75f, 0.01f, 100.0f));
		mComponents.push_back(mCamera.get());
		mServices.AddService(Camera::TypeIdClass(), mCamera.get());

		try
		{
			//mTexturedModelDemo = std::unique_ptr<TexturedDemo>(new TexturedDemo(*this, *mCamera));
			//mComponents.push_back(mTexturedModelDemo.get());

			//mDiffuseCube = std::unique_ptr<DiffuseCube>(new DiffuseCube(*this, *mCamera, "Cube.obj.txt", "Cube.mtl.txt"));
			//mComponents.push_back(mDiffuseCube.get());
			
			mUFOSpecularModel = std::unique_ptr<SpecularModel>(new SpecularModel(*this, *mCamera, "UFOSaucer3.jpg", "UFOSaucer.obj.txt.zip", "UFOSaucer.mtl.txt"));
			mComponents.push_back(mUFOSpecularModel.get());

			//mStarModel = std::unique_ptr<StarModel>(new StarModel(*this, *mCamera, glm::vec3(1.0f,0.0f,1.0f), "Star.obj.txt", "Star.mtl.txt"));
			//mComponents.push_back(mStarModel.get());
		}
		catch (SceneException& e)
		{
			char buf[1000];
			SPRINTF(buf, "ModelEffect exception:%s", e.what());
			gLogger.DebugPrint(buf);
		}

#ifdef __EMSCRIPTEN__
		gDownloadSingleton.DownloadAllNow();
#endif

	}

	void RenderingScene::Initialize()
	{
		Scene::Initialize();

		mCamera->SetPosition(0.0f, 0.0f, 2.0f);
	}

	void RenderingScene::Shutdown()
	{
		Scene::Shutdown();
	}

	void RenderingScene::Draw(const SceneTime& gameTime)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		float grey = 35.0f / 255.0f;
		glClearColor(grey, grey, grey, 1.0f);

		Scene::Draw(gameTime);

		SDL_GL_SwapWindow(mWindow);

#ifdef VIDEO_ENCODER
		static GLfloat start_time = gameTime.TotalGameTime();
		GLfloat elapsed_time = gameTime.TotalGameTime() - start_time;
		if(elapsed_time > 5.0f) // During video encoding, only run for 5 seconds.
		{
			Scene::setVideoEnded();
		}
#endif

	}
}