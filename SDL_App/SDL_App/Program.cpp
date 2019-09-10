#include "Common.h"
#include "RenderingScene.h"
#include "SceneException.h"
#include "Texture.h"
#include "ConfigSingleton.h"
#include <DownloadSingleton.h>
#include <VariaNetDebugPrint.h>
#include <constants.h>
#include "StringUtils.h"

#ifdef VIDEO_ENCODER
	#include "../SDL_App_DLL/Program.h"
#endif // VIDEO_ENCODER

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef _WIN32
	#if defined(DEBUG) || defined(_DEBUG)
		#define _CRTDBG_MAP_ALLOC
		#include <stdlib.h>
		#include <crtdbg.h>
	#endif
#endif

using namespace Library;
using namespace Rendering;

#ifdef __EMSCRIPTEN__
	#define SRC_FOLDER "https://localhost:44319/"
#else
	#define SRC_FOLDER ".\\"
#endif

DownloadSingleton gDownloadSingleton(SRC_FOLDER);
ConfigSingleton gConfigSingleton;
Logger gLogger;

#if defined(__EMSCRIPTEN__)
RenderingScene* g_RenderingScene = NULL;
void emscripten_main();
int main()
{
	gConfigSingleton.DownloadFiles();
	emscripten_set_main_loop(emscripten_main, 0, 0);
}
void emscripten_main()
{
	try
	{
		if (gConfigSingleton.IsInitialized() == false)
		{
			return;
		}

		Texture::setScreenDim(gConfigSingleton.GetScreenWidth(), gConfigSingleton.GetScreenHeight());
		RenderingScene* renderingScene = new RenderingScene(L"#canvas", gConfigSingleton.GetScreenWidth(), gConfigSingleton.GetScreenHeight());
		g_RenderingScene = renderingScene;

		renderingScene->Run();
	}
	catch (SceneException& ex)
	{
		printf("SceneException:%s\n", ex.GetError().c_str());
	}
	catch (std::exception& ex)
	{
		printf("Std Exception:%s\n", ex.what());
	}
}
#elif defined(VIDEO_ENCODER)
SDL_APP_DLL_API int WINAPI check_project_file(const wchar_t* file, int* width, int* height, int* fps)
{
	ConfigSingleton config;
	const std::string afile = toAString(file);
	bool ret = config.LoadConfigFile(afile);
	if (ret)
	{
		*width = config.GetScreenWidth();
		*height = config.GetScreenHeight();
		*fps = config.GetFPS();
	}
	return ret;
}
SDL_APP_DLL_API int WINAPI encoder_main(UINT** pixels, HANDLE evtRequest, HANDLE evtReply, HANDLE evtExit, HANDLE evtVideoEnded, const WCHAR* szUrl)
{
#ifdef _WIN32
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

	try
	{
		const std::string& config_file = toAString(szUrl);

		gConfigSingleton.OpenFile(config_file.c_str(), Library::DownloadableComponent::FileType::INI_FILE);
		if (gConfigSingleton.IsLoadSuccess() == false)
		{
			gLogger.Log("gConfigSingleton.IsLoadSuccess() failed! See log!");

			return 1;
		}

		Texture::setScreenDim(gConfigSingleton.GetScreenWidth(), gConfigSingleton.GetScreenHeight());
		std::unique_ptr<RenderingScene> renderingScene(new RenderingScene(L"Photo Montage", gConfigSingleton.GetScreenWidth(), gConfigSingleton.GetScreenHeight()));
		renderingScene->initVideoEncoder(pixels, evtRequest, evtReply, evtExit, evtVideoEnded, gConfigSingleton.GetFPS());

		renderingScene->Run();
	}
	catch (SceneException& ex)
	{
		gLogger.Log(ex.GetError().c_str());
	}
	catch (std::exception& ex)
	{
		gLogger.Log(ex.what());
	}

	return 0;
}
#else
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
#ifdef _WIN32
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

	try
	{
		gConfigSingleton.DownloadFiles();
		if (gConfigSingleton.IsLoadSuccess() == false)
		{
			const std::string error = "Load project file failed! See log!";
			gLogger.Log(error.c_str());

			::MessageBoxA(NULL, error.c_str(), NULL, MB_OK);

			return 1;
		}

		Texture::setScreenDim(gConfigSingleton.GetScreenWidth(), gConfigSingleton.GetScreenHeight());
		std::unique_ptr<RenderingScene> renderingScene(new RenderingScene(L"Photo Montage", gConfigSingleton.GetScreenWidth(), gConfigSingleton.GetScreenHeight()));

		renderingScene->Run();
	}
	catch (SceneException& ex)
	{
		gLogger.Log(ex.GetError().c_str());
	}
	catch (std::exception& ex)
	{
		gLogger.Log(ex.what());
	}

	return 0;
}
#endif