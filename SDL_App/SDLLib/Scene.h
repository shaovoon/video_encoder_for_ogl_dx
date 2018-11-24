/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#pragma once

#include "Common.h"
#include "SceneClock.h"
#include "SceneTime.h"
#include "SceneComponent.h"
#include <functional>
#include <SDL.h>
#include <SDL_video.h>
#include <SDL_image.h> //Needed for IMG_Load.  If you want to use bitmaps (SDL_LoadBMP), it appears to not be necessary

namespace Library
{
	class Scene : public RTTI
	{
		RTTI_DECLARATIONS(Scene, RTTI)

	public:
		typedef std::function<void(int, int, int, int)> KeyboardHandler;
		
		Scene(const std::wstring& windowTitle, unsigned int screenWidth, unsigned int screenHeight);

		~Scene();

		SDL_Window* Window() const;
#ifdef _WIN32
		HWND Scene::WindowHandle() const;
#endif

#ifdef VIDEO_ENCODER
		void initVideoEncoder(UINT** pixels, HANDLE evtRequest, HANDLE evtReply, HANDLE evtExit, HANDLE evtVideoEnded, int fps);
#endif // VIDEO_ENCODER

		bool DepthBufferEnabled() const;
		const std::wstring& WindowClass() const;
		const std::wstring& WindowTitle() const;
		int ScreenWidth() const;
		int ScreenHeight() const;
		float AspectRatio() const;
		bool IsFullScreen() const;
		const std::vector<SceneComponent*>& Components() const;
		const ServiceContainer& Services() const;
		bool EnableVSync(bool enable);
		void EnableBlend();
		
		virtual void Run();

		void Render(bool& quit);

		virtual void Exit();
		virtual void Initialize();
		virtual void Update(const SceneTime& gameTime);
		virtual void Draw(const SceneTime& gameTime);

		virtual bool IsAllReady() { return false; }
		virtual void CreateComponents() {}
		virtual bool IsAllInitialized() { return false; }

		void SetPause(bool pause) { mPaused = pause; }
		bool GetPause() const { return mPaused; }

		static Scene* sInternalInstance;

	protected:
		virtual void InitializeWindow();
		virtual void InitializeOpenGL();
		virtual void Shutdown();

		void CreateFBO();
		void ReadBuffer(bool& quit);

		std::wstring mWindowTitle;		
		SDL_Window* mWindow;
		bool mHideWindow;
		unsigned int mScreenWidth;
		unsigned int mScreenHeight;
		bool mIsFullScreen;

		GLint mMajorVersion;
		GLint mMinorVersion;

		bool mDepthBufferEnabled;
		bool mStencilBufferEnabled;

		SceneClock mGameClock;
		SceneTime mGameTime;

		std::vector<SceneComponent*> mComponents;
		ServiceContainer mServices;

		SDL_GLContext mContext;
		bool mPaused;


#ifdef VIDEO_ENCODER
		static unsigned int determineMinDim(unsigned int width, unsigned int height);
		void setVideoEnded() { SetEvent(mEvtVideoEnded); }

		GLuint mTexture;
		GLuint mRenderBuffer;
		GLuint mDepthBuffer;
		GLuint mMultisampleTexture;
		UINT** mPixels;
		unsigned int* mPixelBuffer;
		HANDLE mEvtRequest; 
		HANDLE mEvtReply;
		HANDLE mEvtExit;
		HANDLE mEvtVideoEnded;
#endif // VIDEO_ENCODER

	private:
		Scene(const Scene& rhs);
		Scene& operator=(const Scene& rhs);

#ifdef _WIN32
		POINT CenterWindow(int windowWidth, int windowHeight);
#endif
	};
}