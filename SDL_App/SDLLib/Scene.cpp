/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#include "Scene.h"
#include "DrawableSceneComponent.h"
#include "SceneException.h"
#include "Utility.h"
#include "SDL_syswm.h"
#include "constants.h"
#include "timer.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void EmscriptenRender();
void EmptyRender();

namespace Library
{
	RTTI_DEFINITIONS(Scene)

	Scene* Scene::sInternalInstance = nullptr;

	Scene::Scene(const std::wstring& windowTitle, unsigned int screenWidth, unsigned int screenHeight)
		: mWindow(nullptr), mHideWindow(false), mWindowTitle(windowTitle),
		mScreenWidth(screenWidth), mScreenHeight(screenHeight), mIsFullScreen(false),
		mMajorVersion(0), mMinorVersion(0),
		mGameClock(), mGameTime(), mComponents(), mServices(),
		mDepthBufferEnabled(true), mStencilBufferEnabled(false), mPaused(false)
	{
		GlobalServices.AddService(TypeIdClass(), &(*this));
#ifdef VIDEO_ENCODER
		mHideWindow = false; // TODO: set to true for video encoder
#endif

#ifdef __EMSCRIPTEN__
		sInternalInstance = nullptr;
		emscripten_cancel_main_loop();
		emscripten_set_main_loop(EmscriptenRender, 0, 0);
#endif
	}

#ifdef VIDEO_ENCODER
	void Scene::initVideoEncoder(UINT** pixels, HANDLE evtRequest, HANDLE evtReply, HANDLE evtExit, HANDLE evtVideoEnded, int fps)
	{
		mTexture = 0; mRenderBuffer = 0; mDepthBuffer = 0; mMultisampleTexture = 0; mPixels = pixels; mPixelBuffer = nullptr;
		mEvtRequest = evtRequest; mEvtReply = evtReply; mEvtExit = evtExit; mEvtVideoEnded = evtVideoEnded;

		mGameClock.setTimeQuandant((1000.0f/(float)fps)/1000.0f);
	}
#endif // VIDEO_ENCODER

	Scene::~Scene()
	{
		mComponents.clear();
#ifdef VIDEO_ENCODER
		if (mPixelBuffer)
		{
			delete [] mPixelBuffer;
			mPixelBuffer = nullptr;
		}
		glDeleteRenderbuffers(1, &mRenderBuffer);
		glDeleteRenderbuffers(1, &mDepthBuffer);
		glDeleteTextures(1, &mTexture);
		glDeleteTextures(1, &mMultisampleTexture);
#endif
	}

	SDL_Window* Scene::Window() const
	{
		return mWindow;
	}

#ifdef _WIN32
	HWND Scene::WindowHandle() const
	{
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(mWindow, &wmInfo);
		HWND hwnd = wmInfo.info.win.window;
		return hwnd;
	}
#endif

	const std::wstring& Scene::WindowTitle() const
	{
		return mWindowTitle;
	}

	int Scene::ScreenWidth() const
	{
		return mScreenWidth;
	}

	int Scene::ScreenHeight() const
	{
		return mScreenHeight;
	}

	float Scene::AspectRatio() const
	{
		return static_cast<float>(mScreenWidth) / mScreenHeight;
	}
	
	bool Scene::IsFullScreen() const
	{
		return mIsFullScreen;
	}

	const std::vector<SceneComponent*>& Scene::Components() const
	{
		return mComponents;
	}

	const ServiceContainer& Scene::Services() const
	{
		return mServices;
	}

	void Scene::Run()
	{
		InitializeWindow();
		InitializeOpenGL();
		CreateComponents();

		mGameClock.Reset();

		sInternalInstance = this;

#ifndef __EMSCRIPTEN__
		//Main loop flag
		bool quit = false;

		//Event handler
		SDL_Event e;
		//Enable text input
		SDL_StartTextInput();
		//While application is running
		while (!quit)
		{
			//Handle events on queue
			while (SDL_PollEvent(&e) != 0)
			{
				//User requests quit
				if (e.type == SDL_QUIT)
				{
					quit = true;
				}
				//Handle keypress with current mouse position
				else if (e.type == SDL_TEXTINPUT)
				{
					//int x = 0, y = 0;
					//SDL_GetMouseState(&x, &y);
					//handleKeys(e.text.text[0], x, y);

					if (e.text.text[0] == 'q' || e.text.text[0] == 'Q')
						quit = true;
				}
			}
			Render(quit);
		}

		Shutdown();
#endif
	}

	void Scene::Exit()
	{
	}

	void Scene::Initialize()
	{
		for (SceneComponent* component : mComponents)
		{
			component->Initialize();
		}
	}

	void Scene::Update(const SceneTime& gameTime)
	{
		for (SceneComponent* component : mComponents)
		{
			if (component->Enabled())
			{
				component->Update(gameTime);
			}
		}
	}

	void Scene::Draw(const SceneTime& gameTime)
	{
		for (SceneComponent* component : mComponents)
		{
			DrawableSceneComponent* drawableGameComponent = component->As<DrawableSceneComponent>();
			if (drawableGameComponent != nullptr && drawableGameComponent->Visible())
			{
				drawableGameComponent->Draw(gameTime);
			}
		}
	}

	void Scene::InitializeWindow()
	{
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			throw SceneException(MY_FUNC, "SDL_Init() failed.");
		}

		//Use OpenGL 2.1
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

#ifndef __EMSCRIPTEN__
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
#endif
		if (mStencilBufferEnabled)
		{
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
		}

		//Create window
		mWindow = SDL_CreateWindow("Movie", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, mScreenWidth, mScreenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		if (mWindow == NULL)
		{
			char buf[256];
			SPRINTF(buf, "Window could not be created! SDL Error: %s\n", SDL_GetError());
			Shutdown();
			throw SceneException(MY_FUNC, buf);
		}
		if (mHideWindow)
		{
			SDL_HideWindow(mWindow);
		}
#ifdef _WIN32
		POINT center = CenterWindow(mScreenWidth, mScreenHeight);

		SDL_SetWindowPosition(mWindow, center.x, center.y);
#endif
	}

	void Scene::InitializeOpenGL()
	{
		//Create context
		mContext = SDL_GL_CreateContext(mWindow);
		if (mContext == NULL)
		{
			char buf[256];
			SPRINTF(buf, "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
			throw SceneException(MY_FUNC, buf);
		}



#ifdef VIDEO_ENCODER
		bool vsync = false;
#else
		bool vsync = true;
#endif

		if (!EnableVSync(vsync))
		{
			char buf[256];
			SPRINTF(buf, "Warning: Unable to set VSync to %d! SDL Error: %s\n", vsync, SDL_GetError());
			throw SceneException(MY_FUNC, buf);
		}

		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			char buf[256];
			SPRINTF(buf, "GLEW init failed: %s!\n", glewGetErrorString(err));
			throw SceneException(MY_FUNC, buf);
		}

#ifndef __EMSCRIPTEN__
		glGetIntegerv(GL_MAJOR_VERSION, &mMajorVersion);
		glGetIntegerv(GL_MINOR_VERSION, &mMinorVersion);
#endif

		if (mDepthBufferEnabled)
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
		}

#ifndef __EMSCRIPTEN__
		// Enable multisampling
		glEnable(GL_MULTISAMPLE);
#endif
		glViewport(0, 0, mScreenWidth, mScreenHeight);

	}

	void Scene::Shutdown()
	{
		SDL_DestroyWindow(mWindow);

		mWindow = NULL;

		SDL_Quit();
	}

#ifdef _WIN32
	POINT Scene::CenterWindow(int windowWidth, int windowHeight)
	{
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		POINT center;
		center.x = (screenWidth - windowWidth) / 2;
		center.y = (screenHeight - windowHeight) / 2;

		return center;
	}
#endif

	bool Scene::EnableVSync(bool enable)
	{
		return SDL_GL_SetSwapInterval(enable) == 0;
	}

	void Scene::EnableBlend()
	{
		glEnable(GL_BLEND);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//glBlendEquation
	}

	void Scene::Render(bool& quit)
	{
		static bool all_init = false;
		if (all_init == false)
		{
			if (IsAllReady())
			{
				printf("%s\n", "Render2");
				timer stopwatch;
				stopwatch.start("Initialize time");
				CreateFBO();
				Initialize();
				stopwatch.stop();
				if (IsAllInitialized())
				{
					printf("%s\n", "Rendering starts...");
#ifdef __EMSCRIPTEN__
					/*
					EM_ASM(var load_ele = document.getElementById("loading"); 
					if (load_ele)
					{
						load_ele.hidden = true;
						load_ele.style.display = "none";
					}
					document.getElementById("MyMusic").play(); 
					);
					*/
#endif
					all_init = true;
					mGameClock.Reset();
				}
			}
		}
		if (all_init)
		{
			mGameClock.SetPause(mPaused);
			if (mPaused == false)
			{
				mGameClock.UpdateGameTime(mGameTime);
			}
			Update(mGameTime);
			Draw(mGameTime);
			ReadBuffer(quit);
		}
	}

	void Scene::CreateFBO()
	{
#ifdef VIDEO_ENCODER
		mPixelBuffer = new unsigned int[mScreenWidth*mScreenHeight];

		glGenTextures(1, &mMultisampleTexture);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mMultisampleTexture);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, mScreenWidth, mScreenHeight, GL_TRUE);

		glGenRenderbuffers(1, &mRenderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, mRenderBuffer);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 16, GL_RGBA8, mScreenWidth, mScreenHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mRenderBuffer);

		// Create depth render buffer (This is optional)
		glGenRenderbuffers(1, &mDepthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, mDepthBuffer);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 16, GL_DEPTH24_STENCIL8, mScreenWidth, mScreenHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthBuffer);

		GLuint mTexture = 0;
		glGenTextures(1, &mTexture);
		glBindTexture(GL_TEXTURE_2D, mTexture);
		unsigned int dim = determineMinDim(mScreenWidth, mScreenHeight);
		unsigned int* pixels = new unsigned int[dim * dim];
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dim, dim, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		delete [] pixels;
		pixels = NULL;

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);

		// Enable multisampling
		glEnable(GL_MULTISAMPLE);

		if (GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER))
		{
			OutputDebugStringA("FBO status: Complete!\n");
		}
		else
		{
			OutputDebugStringA("FBO status: Not complete!\n");
		}
#endif // VIDEO_ENCODER
	}

	void Scene::ReadBuffer(bool& quit)
	{
#ifdef VIDEO_ENCODER
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		glReadPixels(0, 0, mScreenWidth, mScreenHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, mPixelBuffer);

		HANDLE arrHandle[2];
		arrHandle[0] = mEvtRequest;
		arrHandle[1] = mEvtExit;

		DWORD dwEvt = WaitForMultipleObjects(2, arrHandle, FALSE, INFINITE);

		if (dwEvt == WAIT_OBJECT_0 + 1)
		{
			quit = true;
		}
		while (*mPixels == NULL) { Sleep(100); }

		if(*mPixels)
			memcpy(*mPixels, mPixelBuffer, mScreenWidth*mScreenHeight*sizeof(unsigned int));

		SetEvent(mEvtReply);
#endif // VIDEO_ENCODER
	}

#ifdef VIDEO_ENCODER
	unsigned int Scene::determineMinDim(unsigned int width, unsigned int height)
	{
		unsigned int dim = width;
		if (height > width)
			dim = height;

		unsigned int min_dim = 32;
		if (dim > 32 && dim <= 64)
			min_dim = 64;
		else if (dim > 64 && dim <= 128)
			min_dim = 128;
		else if (dim > 128 && dim <= 256)
			min_dim = 256;
		else if (dim > 256 && dim <= 512)
			min_dim = 512;
		else if (dim > 512 && dim <= 1024)
			min_dim = 1024;
		else if (dim > 1024 && dim <= 2048)
			min_dim = 2048;
		else if (dim > 2048 && dim <= 4096)
			min_dim = 4096;
		else
			min_dim = 4096;

		return min_dim;
	}
#endif // VIDEO_ENCODER
}

void EmscriptenRender()
{
	if (Library::Scene::sInternalInstance)
	{
		bool quit = false;
		Library::Scene::sInternalInstance->Render(quit);
	}
}

void EmptyRender()
{
}
