# H264 Video Encoder for OpenGL and DirectX

Writing documentation in process. Do not read or clone this repo now.

## Requirements

_Video Encoder_
* Visual C++ 2015/2017
* Windows 7/8/10

_OpenGL Renderer_
* SDL2
* SDL2 Image
* GLEW
* GLM
* Obj loader
* Zlib

All the OpenGL Renderer required libraries are included in the repository. The focus is on _Video Encoder_.

## Introduction

I worked on this video encoder while writing my Windows Store App, Mandy Frenzy, a photo slideshow app for ladies. Right now, I am feeling burnt out so I am taking a short hiatus. Meanwhile I write a series of short articles as a way to document this app. This video encoder is header file only (H264Writer.h), based on Microsoft Media Foundation, not the old DirectShow and it is tested on Windows 10. However, it should work fine on Windows 7/8 as well.

What are the pros and cons of this encoder over FFmpeg?

FFmpeg is GPL and so you may not concerned if you just want to encode the output of your personal renderer. For my freemium app, I prefer to steer clear of the licensing issues. How hobbyist usually encode their frames with FFmpeg is to save all the frames in HDD first which limits the number of frames and also directly impacted video duration that can be saved depending on the free HDD space. The extra step of saving and opening the files has negative impact of the encoding speed. Of course, tight integration with FFmpeg code may eliminate the frame saving part. On the other hand, this encoder reads RGB values from the framebuffer provided. The downside is it is not portable and only works on Windows 7/8/10.

The same OpenGL renderer can be compiled into 3 modes: normal OpenGL display mode, Video Encoder mode and Emscripten mode. The latter two's code sections are respectively guarded by VIDEO_ENCODER and __EMSCRIPTEN__ macros. You can, by all means, use your own renderer with the video encoder. The default OpenGL renderer is just provided to show a working demo.

The documentation is divided into 3 main sections. First section is get the demo up and running and on how to modify the parameters. Second section is on how to integrate it with your OpenGL framework. The demo uses a renderer framework used in Paul Varcholik's OpenGL Essentials LiveLessons. A tutorial on how to integrate with DirectX comes later. In theory, this video encoder should integrate well with other graphics API like Vulkan, afterall, all it needs to be supplied with a video buffer and some synchronization in tandem to perform its work. Third section (empty) is on the explanation of the internals of the video encoder. And the last section explains the Emscripten part required to compile into asm.js or Webassembly.

## Running the Demo

All the required libraries are included in the repository. The required dlls are copied automatically to the Release or Debug folder for Win32 post builds. x64 build is unbuildable due to inability to find a x64 zlib lib/dll on the web; this is a linking problem lies with the OpenGL renderer, not video encoder.

To see the OpenGL demo, open up SDL_App.sln in Visual Studio and build the SDL_App project

To run the video encoding demo, open up H264SinkWriter.cpp and in the main function, modify the configFile, musicFile and videoFile to the paths on your machine. configFile is found in the $(SolutionDir)SDL_App folder. musicFile should be a mp3 file and if it is left blank, the final video shall not have music. videoFile is the encoded video. You can specify HVEC(aka H265) instead of H264 in the 4th parameter. HVEC is having some encoding issues where the colors bleed.

```Cpp
// This is the config file to be found in SDL_App project folder.
std::wstring configFile(L"D:\\GitHub\\video_encoder_for_ogl_dx\\SDL_App\\SDL_App\\config.txt");
// This is your music file
std::wstring musicFile(L"D:\\FMA.mp3");
// This is the video encoded output file.
std::wstring videoFile(L"C:\\Users\\shaov\\Documents\\video.mp4");

H264Writer writer(musicFile.c_str(), configFile.c_str(), videoFile.c_str(), VideoCodec::H264);
if (writer.IsValid())
{
    if (writer.Process())
    {
        printf("Video written successfully!\n");
        return 0;
    }
}
printf("Video write failed!\n");
getchar();
```

The typical config.txt is to facilitate passing of information to OpenGL renderer, has nothing to do with video encoder. If your renderer can get the information about the video it is about to encode, then just pass a dummy config.txt. The contents of a typical config.txt is shown below.

```
ScreenWidth=800
ScreenHeight=600
LogPath=C:\Users\shaov\Documents\log.txt
FPS=60
```

Now the demo does not handle aspect ratio and it always stick with 4:3 ratio. If you enter anything which is 16:9, or wider than 4:3, in screen width and height, your video will look stretched. FPS entry is for the integer number of frames per second; there is no way to enter a decimal number like 29.7777.

The demo would only encode 5 seconds of the video. Change duration in RenderingScene::Draw function.

```Cpp
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
    // During video encoding, only run for 5 seconds.
    if(elapsed_time > 5.0f) 
    {
        Scene::setVideoEnded();
    }
#endif
}
```

You have to experiment to find out the optimal bitrate that can encode a good quality video. 4000000 is overkill.

```Cpp
enum class VideoCodec
{
    H264,
    HVEC
};

// H264Writer constructor
H264Writer(const wchar_t* mp3_file, const wchar_t* src_file, const wchar_t* dest_file, 
           VideoCodec codec, UINT32 bitrate = 4000000) :
{...}
```

To run the demo executable by itself, you need to copy the config.txt, Images and Models folders to the Release/Debug folder. The SDL2 dlls would have already copied during post build.

By default, demo displays a rotating UFO saucer, to display other 3D model, just uncomment the other lines in CreateComponents(). 

```Cpp
void RenderingScene::CreateComponents()
{
    mCamera = std::unique_ptr<FirstPersonCamera>(new 
        FirstPersonCamera(*this, 45.0f, 1.0f / 0.75f, 0.01f, 100.0f));
    mComponents.push_back(mCamera.get());
    mServices.AddService(Camera::TypeIdClass(), mCamera.get());

    try
    {
        //mTexturedModelDemo = std::unique_ptr<TexturedDemo>(new TexturedDemo(*this, *mCamera));
        //mComponents.push_back(mTexturedModelDemo.get());

        //mDiffuseCube = std::unique_ptr<DiffuseCube>(new DiffuseCube(*this, *mCamera, 
        //    "Cube.obj.txt", "Cube.mtl.txt"));
        //mComponents.push_back(mDiffuseCube.get());
        
        mUFOSpecularModel = std::unique_ptr<SpecularModel>(new SpecularModel(*this, *mCamera, 
            "UFOSaucer3.jpg", "UFOSaucer.obj.txt.zip", "UFOSaucer.mtl.txt"));
        mComponents.push_back(mUFOSpecularModel.get());

        //mStarModel = std::unique_ptr<StarModel>(new StarModel(*this, *mCamera, 
        //    glm::vec3(1.0f,1.0f,0.0f), "Star.obj.txt", "Star.mtl.txt"));
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
```

## Integration with your OpenGL Framework

This section teaches the modification needed to integrate the video encoder into your renderer.

The fastest way to find all the encoding related code is to search for VIDEO_ENCODER macro in the source code. The encoder requires you to implement 2 global functions: check_config_file and encoder_start. See their declarations below. encoder_start is called in the worker thread.

```Cpp
extern int check_config_file(const wchar_t* file, int* width, int* height, int* fps);
extern int encoder_start(UINT** pixels, HANDLE evtRequest, HANDLE evtReply, 
                         HANDLE evtExit, HANDLE evtVideoEnded, const WCHAR* szUrl);
```

They in turn called their dll counterparts implemented in the Program.cpp

```Cpp
int check_config_file(const wchar_t* file, int* width, int* height, int* fps)
{
    return ::check_project_file(file, width, height, fps);
}
int encoder_start(UINT** pixels, HANDLE evtRequest, HANDLE evtReply, 
                  HANDLE evtExit, HANDLE evtVideoEnded, const WCHAR* szUrl)
{
    return ::encoder_main(pixels, evtRequest, evtReply, evtExit, 
                          evtVideoEnded, szUrl);
}
```

check_project_file requires you to pass the screen width, height and FPS information from the file which is config.txt. In reality, you can just open any file which can provide you these information, so implementation of check_project_file is unimportant. Let's see how encoder_main is implemented.

```Cpp
SDL_APP_DLL_API int WINAPI check_project_file(const wchar_t* file, int* width, 
                                              int* height, int* fps)
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

SDL_APP_DLL_API int WINAPI encoder_main(UINT** pixels, HANDLE evtRequest, HANDLE evtReply, HANDLE evtExit, 
                                        HANDLE evtVideoEnded, const WCHAR* szUrl)
{
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
        std::unique_ptr<RenderingScene> renderingScene(new RenderingScene(L"Photo Montage", 
            gConfigSingleton.GetScreenWidth(), gConfigSingleton.GetScreenHeight()));
        renderingScene->initVideoEncoder(pixels, evtRequest, evtReply, evtExit, 
            evtVideoEnded, gConfigSingleton.GetFPS());

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
```

encoder_main is very similar to WinMain except it calls initVideoEncoder function to hand over the parameters. This is how initVideoEncoder is implemented: it just zero initialized some members and save the parameters inside its members. These HANDLE arguments are already initialized inside the H264Writer constructor. setTimeQuandant() is to set the time increment for every frame, for example if FPS is 30, then the time increment should be 33.3, irregardless of actual time passed. You wouldn't want varying time rate for your video encoding.

```Cpp
void Scene::initVideoEncoder(UINT** pixels, HANDLE evtRequest, HANDLE evtReply, 
                             HANDLE evtExit, HANDLE evtVideoEnded, int fps)
{
    mTexture = 0; 
    mRenderBuffer = 0; 
    mDepthBuffer = 0; 
    mMultisampleTexture = 0; 
    mPixels = pixels; 
    mPixelBuffer = nullptr;
    mEvtRequest = evtRequest; 
    mEvtReply = evtReply; 
    mEvtExit = evtExit; 
    mEvtVideoEnded = evtVideoEnded;

    mGameClock.setTimeQuandant((1000.0f/(float)fps)/1000.0f);
}
```

This is how encoder_start is called in the worker thread started by H264Writer

```Cpp
DWORD WINAPI ThreadOpenGLProc(LPVOID pParam)
{
    H264Writer* pWriter = (H264Writer*)(pParam);
    return ::encoder_start((UINT**)(
        pWriter->GetImagePtr()), 
        pWriter->GetRequestEvent(), 
        pWriter->GetReplyEvent(), 
        pWriter->GetExitEvent(), 
        pWriter->GetVideoEndedEvent(), 
        pWriter->GetUrl().c_str());
}
```

In our OpenGL renderer, we need to create RenderBuffer for our renderer to draw on.

```Cpp
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
```

determineMinDim() used in above function is implemented in this way because we need a square texture to power of 2.

```Cpp
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
```

Lastly, we need to read the rendered buffer with glReadPixels.

```Cpp
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
```

This is how CreateFBO() and ReadBuffer() are called in my Render(). CreateFBO() and ReadBuffer() are empty when not compiled in VIDEO_ENCODER mode.

```Cpp
void Scene::Render(bool& quit)
{
    static bool all_init = false;
    if (all_init == false)
    {
        if (IsAllReady())
        {
            CreateFBO();
            Initialize();
            if (IsAllInitialized())
            {
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
```

## How is video encoder written

## Running as asm.js on web browser