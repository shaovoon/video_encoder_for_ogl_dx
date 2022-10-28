# H264/HEVC Video Encoder for OpenGL

[![Mandy Frenzy Video](https://img.youtube.com/vi/ilSeZznzDt4/0.jpg)](https://www.youtube.com/watch?v=od1Z9nb5vwQ)

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

All the OpenGL Renderer required libraries are included in the repository. This is a header only video encoder for Windows. You absolutely do not need the included OpenGL framework to use it. The OpenGL framework is to show you on how to integrate it with your OpenGL framework. All the video encoder needs from your side is a image buffer (to read from) and a few windows synchronization primitives. The focus of this README is on _Video Encoder_.

## Introduction

I worked on this video encoder while writing my Windows Store App, Mandy Frenzy, a photo slideshow app for ladies. Right now, I am feeling burnt out so I am taking a short hiatus. Meanwhile I write a series of short articles as a way to document this app. This video encoder is header file only (H264Writer.h), based on Microsoft Media Foundation, not the old DirectShow due to Microsoft did not expose the H264 and HEVC codec on DirectShow. It is tested on Windows 10. It should work fine on Windows 7/8 as well. Do let me know if you encounter any problem on those OSes. Windows 10 used to come bundled with HEVC codec. For unknown reasons, MS has taken it out in Windows 10 Fall Creators Update for the **new** Windows 10 installation and [put it up for purchase](https://www.microsoft.com/en-us/p/hevc-video-extensions/9nmzlz57r3t7?activetab=pivot:overviewtab) for $1.50 in the Microsoft Store. In the section below, some screenshots will show encoding artifacts present in MS HEVC video.

**What are the pros and cons of this encoder over FFmpeg?**

[FFmpeg](https://www.ffmpeg.org/) is GPL and so you may not concerned if you just want to encode the output of your personal renderer. For my freemium app, I prefer to steer clear of the licensing issues. How hobbyist usually encode their frames with FFmpeg is to save all the frames in HDD first which limits the number of frames and also directly impacted video duration that can be saved depending on the free HDD space. The extra step of saving and opening the files has negative impact of the encoding speed. Of course, tight integration with FFmpeg code may eliminate the frame saving part. On the other hand, this encoder reads RGB values from the framebuffer provided. The downside is it is not portable and only works on Windows 7/8/10.

**3 rendering modes**

The same OpenGL renderer can be compiled into 3 modes: normal OpenGL display mode, Video Encoder mode and Emscripten mode. The latter two's code sections are respectively guarded by VIDEO_ENCODER and EMSCRIPTEN macros. You can, by all means, use your own renderer with the video encoder. The default OpenGL renderer is just provided to show a working demo.

The documentation is divided into 3 main sections. First section is get the demo up and running and on how to modify the parameters. Second section is on how to integrate it with your OpenGL framework. The demo uses a renderer framework taught in [Paul Varcholik's OpenGL Essentials LiveLessons](http://www.informit.com/store/opengl-essentials-livelessons-video-training-downloadable-9780133824148). The [original source code](https://bitbucket.org/pvarcholik/opengl-essentials-livelessons) used GLFW and is based on OpenGL 4.4: I converted his framework to use SDL and downgrade to OpenGL 2.0. The decision is based on the lowest denominator of what WebGL and Emscripten can support. A tutorial on how to integrate with DirectX will come later. In theory, this video encoder should integrate well with other graphics API like Vulkan, afterall, all it needs to be supplied with a video buffer and some synchronization in tandem to perform its work. Third section is on the explanation of the internals of the video encoder which you can skipped if you are not into the encoder internals and implementation. And the last section explains the Emscripten part required to compile into asm.js or Webassembly.

## Running the Demo

**Spaceship video**

[Youtube demo](https://www.youtube.com/watch?v=IbZ1_FdEf7g)

![Image of Spaceship](https://github.com/shaovoon/video_encoder_for_ogl_dx/blob/master/images/spaceship.jpg)

All the required libraries are included in the repository. The required dlls are copied automatically to the Release or Debug folder for Win32 post builds. x64 build is unbuildable due to inability to find a x64 zlib lib/dll on the web; this is a linking problem lies with the OpenGL renderer, not video encoder.

To see the OpenGL demo, open up SDL_App.sln in Visual Studio and build the SDL_App project

To run the video encoding demo, open up H264SinkWriter.cpp and in the main function, modify the configFile, musicFile and videoFile to the paths on your machine. configFile is found in the $(SolutionDir)SDL_App folder. musicFile should be a mp3 file and if it is left blank, the final video shall not have music. videoFile is the encoded video. You can specify HVEC(aka H265) instead of H264 in the 4th parameter. HVEC is having some encoding issues where the colors bleed(See screenshots below.).

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

You have to experiment to find out the optimal bitrate that can encode a good quality video.

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

## HEVC Artifacts

**H264 video of Mandy Frenzy**

[Youtube demo](https://www.youtube.com/watch?v=PQ2ZgFLSOEM)

![Image of H264](https://github.com/shaovoon/video_encoder_for_ogl_dx/blob/master/images/h264.jpg)


**HEVC video (Artifacts) of Mandy Frenzy**

[Youtube demo](https://www.youtube.com/watch?v=yoqNbxpckgM)

![Image of HEVC](https://github.com/shaovoon/video_encoder_for_ogl_dx/blob/master/images/hevc.jpg)

__Update:__ The HEVC quality problem is fixed with h/w acceleration.

As you can see the sinewave artifacts in HEVC, not present in H264. Increase the bitrate solves the problem at the expense of larger file size. By the way, the sinewave is rendered by triangles, not lines and not by fragment shaders. Reason being lines are usually implemented as 1 pixel wide in OpenGL ES 2.0. Using triangles allows me to control the width/height.

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

Lastly, we need to read the rendered buffer with glReadPixels. The code wait on mEvtRequest and mEvtExit. mEvtRequest is signaled when the encoding thread requests for new frame. Whereas the mEvtExit is signaled during exit. After copying the buffer to mPixels, mEvtReply is signaled to encoding thread mPixels is ready.

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

Media Foundation provides 3 methods to writing a video encoder.

1. Media Session
2. Transcoder API
3. SinkWriter

All these methods comes with their pros and cons. They range from the most difficult/control to ease to use but with little control.

The first method is to construct your own media session which is a topology. This appraoach has the most flexible/control, on the other hand it has the highest difficulty reserved for MF professional because you have to find Media Source, Transform and Media Sink and connect them together. However, you have the flexibility to choose your transform vendor. For those who are not familiar: Media Sources are used to demultiplex the source file. Media Foundation Transforms are used to decode and encode streams.  Media Sinks are used to multiplex the streams and write the multiplexed stream to a file or network. There is one downside to using this method, because you have to write and register your Media Source dll on user computer. In most cases, this does not present a problem. But my UWP app's installation and operation is sandboxed, meaning all the file saves and registry writes are redirected to unknown location. I am not sure if MF can find and instantiate my Media Source dll if all its does is only look at global registry and folders.

The second method is using a Transcoding API which makes it easier to construct the media session for you with the most sensible options chosen for you. This method is out of question for me because I am not transcoding a video file into another format.

The third method, SinkWriter, is the most simple to integrate with your OpenGL render. ~~One very big downside is it always choose Microsoft **software** transform even when there are hardware accelerated ones available on your system, like in my case, I have the Intel and NVidia H264 h/w encoders(See below). This is the approach I chose.~~ I was mislead by the book: the third method can also make use of hardware acceleration in latest 1.0.3 version.


```
H/w encoder
Video Encoder: Intel« Quick Sync Video H.264 Encoder MFT
Video Encoder: NVIDIA H.264 Encoder MFT
```

```
S/w encoder
Video Encoder: H264 Encoder MFT
```

Enumeration of the encoders is done with EnumVideoEncoder function(Shown below).

```Cpp
std::vector<std::wstring> encoders;
if (H264Writer::EnumVideoEncoder(encoders, Processing::Software, VideoCodec::H264))
{
    for (size_t i = 0; i < encoders.size(); ++i)
    {
        printf("Video Encoder: %S\n", encoders[i].c_str());
    }
}
else
{
    printf("H264Writer::EnumVideoEncoder failed!\n");
}
```

Work shall continue on the 1st method to utilize H/w encoder.

To use this header only library, just include H264Writer.h inside your C++ source code. And remember to implement the 2 functions below. file can be any format. check_config_file shall return the frame dimensions and frame rate per second.

In the integration section above, there is an example of how encoder_start() is implemented by encoder_main().

```Cpp
extern int check_config_file(const wchar_t* file, int* width, int* height, int* fps);
extern int encoder_start(UINT** pixels, HANDLE evtRequest, HANDLE evtReply, HANDLE evtExit, HANDLE evtVideoEnded, const WCHAR* szUrl);
```

In H264Writer only constructor, as you can see, it initialize a bunch of members to the default value. There is m_hThread which runs the OpenGL Win32 loop. One characteristic of this encoder is the OpenGL window is running in the foreground. The user will see that window. If your application requirement is not to show user the windows, you have to create a invisible window in encoder_main(). m_evtRequest is for requesting frame from the OpenGL thread, m_evtReply is used by OpenGL to tell the frame is copied and ready. m_evtExit is signaled in case user close the window before video encoding is completed. m_evtVideoEnded is signalled by Scene::setVideoEnded() in the Draw() function.

```Cpp
H264Writer(const wchar_t* mp3_file, const wchar_t* src_file, const wchar_t* dest_file, VideoCodec codec, UINT32 bitrate = 4000000) :
    m_OpenSrcFileSuccess(false),
    m_MP3Filename(mp3_file),
    m_SrcFilename(src_file),
    m_DestFilename(dest_file),
    m_Width(0),
    m_Height(0),
    m_pImage(nullptr),
    m_cbWidth(4 * m_Width),
    m_cbBuffer(m_cbWidth * m_Height),
    m_pBuffer(nullptr),
    m_hThread(INVALID_HANDLE_VALUE),
    m_evtRequest(INVALID_HANDLE_VALUE),
    m_evtReply(INVALID_HANDLE_VALUE),
    m_evtExit(INVALID_HANDLE_VALUE),
    m_evtVideoEnded(INVALID_HANDLE_VALUE),
    m_CoInited(false),
    m_MFInited(false),
    m_pSinkWriter(nullptr),
    m_VideoFPS(60),
    m_FrameDuration(10 * 1000 * 1000 / m_VideoFPS),
    m_VideoBitrate(bitrate),
    m_EncCommonQuality(100),
    m_VideoCodec(codec),
    m_nStreams(0)
{
```

Inside the constructor body, check_config_file is called to return width, height and fps.

```Cpp
int width = 0; int height = 0; int fps = 0;
if (check_config_file(m_SrcFilename.c_str(), &width, &height, &fps))
{
    m_OpenSrcFileSuccess = true;
    m_Width = width;
    m_Height = height;
    m_cbWidth = 4 * m_Width;
    m_cbBuffer = m_cbWidth * m_Height;
    m_VideoFPS = fps;
    m_FrameDuration = (10 * 1000 * 1000 / m_VideoFPS);
    m_pImage = new (std::nothrow) UINT32[m_Width * m_Height];
}

if (!m_OpenSrcFileSuccess)
    return;

if (!m_pImage)
    return;
```

Next, CoInitializeEx is called to initialize the COM runtime before any Media Foundation function can be called. MFStartup() called to initialize the Media Foundation(MF) runtime. After that, we create m_pBuffer based on the dimension stored in m_cbBuffer. This is the real-life example of premature optimization. the buffer is kept inside a member to avoid instantiating and allocating a memory buffer on every frame. It turns out that MFCreateMemoryBuffer does not destroy the buffer in every frame but return the buffer to a pool; whenever a buffer of the same dimension is requested, MF just gives you one of buffers created previously.

```Cpp
HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

if (SUCCEEDED(hr))
{
    m_CoInited = true;
    hr = MFStartup(MF_VERSION);
    if (SUCCEEDED(hr))
    {
        m_MFInited = true;
        // Create a new memory buffer.
        hr = MFCreateMemoryBuffer(m_cbBuffer, &m_pBuffer);
    }
}
```

After IsValid() returns true, we create the Win32 event handle and OpenGL thread.

```Cpp
    if (IsValid() == false)
    {
        return;
    }

    if (SUCCEEDED(hr))
    {
        m_evtRequest = CreateEvent(NULL, FALSE, FALSE, NULL);

        m_evtReply = CreateEvent(NULL, FALSE, FALSE, NULL);

        m_evtExit = CreateEvent(NULL, FALSE, FALSE, NULL);

        m_evtVideoEnded = CreateEvent(NULL, FALSE, FALSE, NULL);

        m_hThread = CreateThread(NULL, 100000, ThreadOpenGLProc, this, 0, NULL);
    }
}

```

Validity is determined by whether COM and MF runtime is initialized successfully.

```Cpp
const bool IsValid() const
{
    return m_CoInited && m_MFInited;
}
```

In the H264Writer destructor, m_evtExit is signalled and sleep for 100 millisecond to ensure the OpenGL thread get the event. The all buffers and handles can be released safely. And MF and COM runtime are shut down.

```Cpp
~H264Writer()
{
    SetEvent(m_evtExit);
    Sleep(100);

    if (m_pImage)
    {
        delete[] m_pImage;
        m_pImage = nullptr;
    }

    m_pBuffer.Release();

    if (m_evtRequest != INVALID_HANDLE_VALUE)
        CloseHandle(m_evtRequest);

    if(m_evtReply != INVALID_HANDLE_VALUE)
        CloseHandle(m_evtReply);

    if(m_evtExit != INVALID_HANDLE_VALUE)
        CloseHandle(m_evtExit);

    if(m_evtVideoEnded!=INVALID_HANDLE_VALUE)
        CloseHandle(m_evtVideoEnded);

    m_pSourceReader = nullptr;
    m_pSinkWriter = nullptr;

    if(m_MFInited)
        MFShutdown();

    if(m_CoInited)
        CoUninitialize();
}
```

GetSourceDuration is used in H264Writer to get the duration of source MP3, through it can be used to duration of any media, including video. Duration are in 10 millionth of a second.

```Cpp
HRESULT GetSourceDuration(IMFMediaSource *pSource, MFTIME *pDuration)
{
    *pDuration = 0;

    IMFPresentationDescriptor *pPD = NULL;

    HRESULT hr = pSource->CreatePresentationDescriptor(&pPD);
    if (SUCCEEDED(hr))
    {
        hr = pPD->GetUINT64(MF_PD_DURATION, (UINT64*)pDuration);
        pPD->Release();
    }
    return hr;
}
```

Here is a short snippet on how to convert duration back to minutes and seconds familiar to humans.

```Cpp
MFTIME total_seconds = duration / 10000000;
MFTIME minute = total_seconds / 60;
MFTIME second = total_seconds % 60;
```

Next function is InitializeWriter

```Cpp
HRESULT InitializeWriter(DWORD *videoStreamIndex, DWORD *audioStreamIndex)
{
    HRESULT hr = S_OK;
    m_pSourceReader = nullptr;
    m_pSinkWriter = nullptr;
    *videoStreamIndex = 1;
    *audioStreamIndex = 0;

    IMFMediaType    *pMediaTypeOut = nullptr;
    IMFMediaType    *pMediaTypeIn = nullptr;
    DWORD           streamIndex=1;
    CComPtr<IMFAttributes> pConfigAttrs;
```

We set the attributes that hardware accelerated transforms are preferred. When there is no hardware transforms installed on the system, software ones will be chosen.

```Cpp
    do
    {
        // create an attribute store
        hr = MFCreateAttributes(&pConfigAttrs, 1);
        BREAK_ON_FAIL(hr);

        // set MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS property in the store
        hr = pConfigAttrs->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
        BREAK_ON_FAIL(hr);
```

When the m_MP3Filename is specified, we create a m_pSourceReader that is able to decode mp3. pSource is used to get and print out the duration. which is useless. So you can delete the pSource line and all the lines below it.

```Cpp
        if (m_MP3Filename.empty() == false)
        {
            // create a source reader
            hr = MFCreateSourceReaderFromURL(m_MP3Filename.c_str(), pConfigAttrs, &m_pSourceReader);
            BREAK_ON_FAIL(hr);

            IMFMediaSource* pSource = nullptr;
            hr = m_pSourceReader->GetServiceForStream(
                MF_SOURCE_READER_MEDIASOURCE,
                GUID_NULL, //MF_MEDIASOURCE_SERVICE,
                IID_IMFMediaSource,
                (void**)&pSource
            );
            BREAK_ON_FAIL(hr);

            MFTIME duration = 0;
            hr = GetSourceDuration(pSource, &duration);
            BREAK_ON_FAIL(hr);
            if (pSource)
                pSource->Release();

            printf("Audio duration:%lld\n", duration);

            MFTIME total_seconds = duration / 10000000;
            MFTIME minute = total_seconds / 60;
            MFTIME second = total_seconds % 60;
            printf("Audio duration:%lld:%lld\n", minute, second);
        }
```

As you can see our do-while is only executed once. The existence of do-while is mainly for BREAK_ON_FAIL macro to break out of it. Next we create a SinkWriter. MapStreams(), SetVideoOutputType() and SetVideoInputType() will be explained shortly.

```Cpp
        hr = MFCreateSinkWriterFromURL(m_DestFilename.c_str(), nullptr, nullptr, &m_pSinkWriter);
        BREAK_ON_FAIL(hr);

        // map the streams found in the source file from the source reader to the
        // sink writer, while negotiating media types
        hr = MapStreams();
        BREAK_ON_FAIL(hr);

        hr = SetVideoOutputType(&pMediaTypeOut, streamIndex);
        BREAK_ON_FAIL(hr);
        hr = SetVideoInputType(&pMediaTypeIn, streamIndex);
        BREAK_ON_FAIL(hr);

        hr = m_pSinkWriter->BeginWriting();
        BREAK_ON_FAIL(hr);

        *videoStreamIndex = streamIndex;
    } while (false);

    SafeRelease(&pMediaTypeOut);
    SafeRelease(&pMediaTypeIn);
    return hr;
}
```

BREAK_ON_FAIL and BREAK_ON_NULL macro are defined in such a way.

```Cpp
#define BREAK_ON_FAIL(value)            if(FAILED(value)) break;
#define BREAK_ON_NULL(value, newHr)     if(value == NULL) { hr = newHr; break; }
```

SetVideoOutputType() is to set the parameters of video output. You can experiment with the values and see how it affect the output. As for what parameters to set, Google is your best friend. I set to the highest quality.

```Cpp
HRESULT SetVideoOutputType(IMFMediaType** pMediaTypeOut, DWORD& streamIndex)
{
    HRESULT hr = S_OK;
    do 
    {
    hr = MFCreateMediaType(pMediaTypeOut);
    BREAK_ON_FAIL(hr);
    hr = (*pMediaTypeOut)->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    BREAK_ON_FAIL(hr);
    if (m_VideoCodec==VideoCodec::HEVC)
    {
        hr = (*pMediaTypeOut)->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_HEVC);
        BREAK_ON_FAIL(hr);
        hr = (*pMediaTypeOut)->SetUINT32(MF_MT_MPEG2_PROFILE, eAVEncH265VProfile_Main_420_8);
        BREAK_ON_FAIL(hr);
    }
    else
    {
        hr = (*pMediaTypeOut)->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
        BREAK_ON_FAIL(hr);
        hr = (*pMediaTypeOut)->SetUINT32(MF_MT_MPEG2_PROFILE, eAVEncH264VProfile_High);
        BREAK_ON_FAIL(hr);
    }


    //hr = pMediaTypeOut->SetUINT32(CODECAPI_AVEncCommonRateControlMode, eAVEncCommonRateControlMode_Quality);
    hr = (*pMediaTypeOut)->SetUINT32(CODECAPI_AVEncCommonRateControlMode, eAVEncCommonRateControlMode_UnconstrainedVBR);
    BREAK_ON_FAIL(hr);
    hr = (*pMediaTypeOut)->SetUINT32(CODECAPI_AVEncCommonQuality, m_EncCommonQuality);
    BREAK_ON_FAIL(hr);
    hr = (*pMediaTypeOut)->SetUINT32(CODECAPI_AVEncVideoOutputColorLighting, eAVEncVideoColorLighting_Office);
    BREAK_ON_FAIL(hr);
    hr = (*pMediaTypeOut)->SetUINT32(CODECAPI_AVEncVideoOutputColorPrimaries, eAVEncVideoColorPrimaries_SameAsSource);
    BREAK_ON_FAIL(hr);
    hr = (*pMediaTypeOut)->SetUINT32(CODECAPI_AVEncVideoOutputColorTransferFunction, eAVEncVideoColorTransferFunction_SameAsSource);
    BREAK_ON_FAIL(hr);
    hr = (*pMediaTypeOut)->SetUINT32(CODECAPI_AVEncCommonQualityVsSpeed, 100); // 100 for quality and 0 for speed
    BREAK_ON_FAIL(hr);
    hr = (*pMediaTypeOut)->SetUINT32(MF_MT_AVG_BITRATE, m_VideoBitrate);
    BREAK_ON_FAIL(hr);
    hr = (*pMediaTypeOut)->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    BREAK_ON_FAIL(hr);
    hr = MFSetAttributeSize((*pMediaTypeOut), MF_MT_FRAME_SIZE, m_Width, m_Height);
    BREAK_ON_FAIL(hr);
    hr = MFSetAttributeRatio((*pMediaTypeOut), MF_MT_FRAME_RATE, m_VideoFPS, 1);
    BREAK_ON_FAIL(hr);
    hr = MFSetAttributeRatio((*pMediaTypeOut), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    BREAK_ON_FAIL(hr);
    hr = m_pSinkWriter->AddStream((*pMediaTypeOut), &streamIndex);
    } while (false);
    return hr;
}
```

SetVideoInputType() is setting input video format which our OpenGL format is RGB32.

```Cpp
const GUID   VIDEO_ENCODING_FORMAT = MFVideoFormat_H264;
const GUID   VIDEO_INPUT_FORMAT = MFVideoFormat_RGB32;

HRESULT SetVideoInputType(IMFMediaType** pMediaTypeIn, DWORD& streamIndex)
{
    HRESULT hr = S_OK;
    do 
    {
        hr = MFCreateMediaType(pMediaTypeIn);
        BREAK_ON_FAIL(hr);
        hr = (*pMediaTypeIn)->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        BREAK_ON_FAIL(hr);
        hr = (*pMediaTypeIn)->SetGUID(MF_MT_SUBTYPE, VIDEO_INPUT_FORMAT);
        BREAK_ON_FAIL(hr);
        hr = (*pMediaTypeIn)->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
        BREAK_ON_FAIL(hr);
        hr = MFSetAttributeSize(*pMediaTypeIn, MF_MT_FRAME_SIZE, m_Width, m_Height);
        BREAK_ON_FAIL(hr);
        hr = MFSetAttributeRatio(*pMediaTypeIn, MF_MT_FRAME_RATE, m_VideoFPS, 1);
        BREAK_ON_FAIL(hr);
        hr = MFSetAttributeRatio(*pMediaTypeIn, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
        BREAK_ON_FAIL(hr);
        hr = m_pSinkWriter->SetInputMediaType(streamIndex, *pMediaTypeIn, NULL);
    } while (false);

    return hr;
}
```

ConnectStream() is attempting to connect the input and output which the source and sink can agree on. This function is mainly boilerplate code.

```Cpp
//
// Attempt to find an uncompressed media type for the specified stream that both the source 
// and sink can agree on
//
HRESULT ConnectStream(DWORD dwStreamIndex,
    const GUID& streamMajorType)
{
    HRESULT hr = S_OK;

    CComPtr<IMFMediaType> pPartialMediaType;
    CComPtr<IMFMediaType> pFullMediaType;

    BOOL fConfigured = FALSE;
    GUID* intermediateFormats = NULL;
    int nFormats = 0;

```

Here comes the familiar do-only-once do-while loop. We set the GUID of the container type.

```Cpp
    do
    {
        // create a media type container object that will be used to match stream input
        // and output media types
        hr = MFCreateMediaType(&pPartialMediaType);
        BREAK_ON_FAIL(hr);

        // set the major type of the partial match media type container
        hr = pPartialMediaType->SetGUID(MF_MT_MAJOR_TYPE, streamMajorType);
        BREAK_ON_FAIL(hr);

        // Get the appropriate list of intermediate formats - formats that every decoder and
        // encoder of that type should agree on.  Essentially these are the uncompressed 
        // formats that correspond to decoded frames for video, and uncompressed audio 
        // formats
        if (streamMajorType == MFMediaType_Video)
        {
            intermediateFormats = intermediateVideoFormats;
            nFormats = nIntermediateVideoFormats;
        }
        else if (streamMajorType == MFMediaType_Audio)
        {
            intermediateFormats = intermediateAudioFormats;
            nFormats = nIntermediateAudioFormats;
        }
        else
        {
            hr = E_UNEXPECTED;
            break;
        }

```

Next, we iterate over every format and find one that can match m_pSourceReader by SetCurrentMediaType(). When it is happy, we can GetCurrentMediaType to get the full media type. If none can match, we set hr to MF_E_INVALIDMEDIATYPE;

```Cpp
        // loop through every intermediate format that you have for this major type, and
        // try to find one on which both the source stream and sink stream can agree on
        for (int x = 0; x < nFormats; x++)
        {
            // set the format of the partial media type
            hr = pPartialMediaType->SetGUID(MF_MT_SUBTYPE, intermediateFormats[x]);
            BREAK_ON_FAIL(hr);

            // set the partial media type on the source stream
            hr = m_pSourceReader->SetCurrentMediaType(
                dwStreamIndex,                      // stream index
                NULL,                               // reserved - always NULL
                pPartialMediaType);                // media type to try to set

                                                   // if the source stream (i.e. the decoder) is not happy with this media type -
                                                   // if it cannot decode the data into this media type, restart the loop in order 
                                                   // to try the next format on the list
            if (FAILED(hr))
            {
                hr = S_OK;
                continue;
            }

            pFullMediaType = NULL;

            // if you got here, the source stream is happy with the partial media type you set
            // - extract the full media type for this stream (with all internal fields 
            // filled in)
            hr = m_pSourceReader->GetCurrentMediaType(dwStreamIndex, &pFullMediaType);

            // Now try to match the full media type to the corresponding sink stream
            hr = m_pSinkWriter->SetInputMediaType(
                dwStreamIndex,             // stream index
                pFullMediaType,            // media type to match
                NULL);                    // configuration attributes for the encoder

                                          // if the sink stream cannot accept this media type - i.e. if no encoder was
                                          // found that would accept this media type - restart the loop and try the next
                                          // format on the list
            if (FAILED(hr))
            {
                hr = S_OK;
                continue;
            }

            // you found a media type that both the source and sink could agree on - no need
            // to try any other formats
            fConfigured = TRUE;
            break;
        }
        BREAK_ON_FAIL(hr);

        // if you didn't match any formats return an error code
        if (!fConfigured)
        {
            hr = MF_E_INVALIDMEDIATYPE;
            break;
        }

    } while (false);

    return hr;
}
```

MapStreams() is for MP3 file, so when it is not specified, we return. m_pSourceReader is for MP3 file. OpenGL source does not need a SourceReader because it is uncompressed and we know its format and there is no video file to read for OpenGL input, unless you can count the config.txt which contains dimension and FPS. MapStreams() is also mostly boilerplate code I copied somewhere.

```Cpp
HRESULT MapStreams(void)
{
    if (m_MP3Filename.empty())
        return S_OK;

    HRESULT hr = S_OK;
    BOOL isStreamSelected = FALSE;
    DWORD sourceStreamIndex = 0;
    DWORD sinkStreamIndex = 0;
    GUID streamMajorType;
    CComPtr<IMFMediaType> pStreamMediaType;
```

The code below add the audio stream to sinkwriter. A media file such as video can contain more than just video and audio, for example subtitle. For encoder, we do not care about media types other than MFMediaType_Audio. Ignore MFMediaType_Video in the code because MapStreams is called for MP3 m_pSourceReader.

```Cpp
do
    {
        m_nStreams = 0;

        while (SUCCEEDED(hr))
        {
            // check whether you have a stream with the right index - if you don't, the 
            // IMFSourceReader::GetStreamSelection() function will fail, and you will drop
            // out of the while loop
            hr = m_pSourceReader->GetStreamSelection(sourceStreamIndex, &isStreamSelected);
            if (FAILED(hr))
            {
                hr = S_OK;
                break;
            }

            // count the total number of streams for later
            m_nStreams++;

            // get the source media type of the stream
            hr = m_pSourceReader->GetNativeMediaType(
                sourceStreamIndex,           // index of the stream you are interested in
                0,                           // index of the media type exposed by the 
                                             //    stream decoder
                &pStreamMediaType);          // media type
            BREAK_ON_FAIL(hr);

            // extract the major type of the source stream from the media type
            hr = pStreamMediaType->GetMajorType(&streamMajorType);
            BREAK_ON_FAIL(hr);

            // select a stream, indicating that the source should send out its data instead
            // of dropping all of the samples
            hr = m_pSourceReader->SetStreamSelection(sourceStreamIndex, TRUE);
            BREAK_ON_FAIL(hr);

            // if this is a video or audio stream, transcode it and negotiate the media type
            // between the source reader stream and the corresponding sink writer stream.  
            // If this is a some other stream format (e.g. subtitles), just pass the media 
            // type unchanged.
            if (streamMajorType == MFMediaType_Audio || streamMajorType == MFMediaType_Video)
            {
                // get the target media type - the media type into which you will transcode
                // the data of the current source stream
                hr = GetTranscodeMediaType(pStreamMediaType);
                BREAK_ON_FAIL(hr);

                // add the stream to the sink writer - i.e. tell the sink writer that a 
                // stream with the specified index will have the target media type
                hr = m_pSinkWriter->AddStream(pStreamMediaType, &sinkStreamIndex);
                BREAK_ON_FAIL(hr);

                // hook up the source and sink streams - i.e. get them to agree on an
                // intermediate media type that will be used to pass data between source 
                // and sink
                hr = ConnectStream(sourceStreamIndex, streamMajorType);
                BREAK_ON_FAIL(hr);
            }
            else
            {
                // add the stream to the sink writer with the exact same media type as the
                // source stream
                hr = m_pSinkWriter->AddStream(pStreamMediaType, &sinkStreamIndex);
                BREAK_ON_FAIL(hr);
            }

            // make sure that the source stream index is equal to the sink stream index
            if (sourceStreamIndex != sinkStreamIndex)
            {
                hr = E_UNEXPECTED;
                break;
            }

            // increment the source stream index, so that on the next loop you are analyzing
            // the next stream
            sourceStreamIndex++;

            // release the media type
            pStreamMediaType = NULL;
        }

        BREAK_ON_FAIL(hr);

    } while (false);

    return hr;
}
```

GetTranscodeAudioType() is to set output audio type. I have forgotten why destination format is MFAudioFormat_AAC, you are welcome to set to MFAudioFormat_MP3.

```Cpp
//
// Get the target audio media type - use the AAC media format.
//
HRESULT GetTranscodeAudioType(
    CComPtr<IMFMediaType>& pStreamMediaType)
{
    HRESULT hr = S_OK;

    do
    {
        BREAK_ON_NULL(pStreamMediaType, E_POINTER);

        // wipe out existing data from the media type
        hr = pStreamMediaType->DeleteAllItems();
        BREAK_ON_FAIL(hr);

        // reset the major type to audio since we just wiped everything out
        pStreamMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
        BREAK_ON_FAIL(hr);

        // set the audio subtype
        hr = pStreamMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
        BREAK_ON_FAIL(hr);

        // set the number of audio bits per sample
        hr = pStreamMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
        BREAK_ON_FAIL(hr);

        // set the number of audio samples per second
        hr = pStreamMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 44100);
        BREAK_ON_FAIL(hr);

        // set the number of audio channels
        hr = pStreamMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 2);
        BREAK_ON_FAIL(hr);

        // set the Bps of the audio stream
        hr = pStreamMediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 16000);
        BREAK_ON_FAIL(hr);

        // set the block alignment of the samples
        hr = pStreamMediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 1);
        BREAK_ON_FAIL(hr);
    } while (false);

    return hr;
}
```

Similarly, GetTranscodeVideoType() is to set output video type.

```Cpp
//
// Get the target video media type - use the H.264 media format.
//
HRESULT GetTranscodeVideoType(
    CComPtr<IMFMediaType>& pStreamMediaType)
{
    HRESULT hr = S_OK;

    do
    {
        BREAK_ON_NULL(pStreamMediaType, E_POINTER);

        // wipe out existing data from the media type
        hr = pStreamMediaType->DeleteAllItems();
        BREAK_ON_FAIL(hr);

        // reset the major type to video since we just wiped everything out
        pStreamMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        BREAK_ON_FAIL(hr);

        // set the video subtype
        if (m_VideoCodec == VideoCodec::H264)
        {
            hr = pStreamMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
        }
        else
        {
            hr = pStreamMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_HEVC);
        }
        BREAK_ON_FAIL(hr);

        // set the frame size to 720p as a 64-bit packed value
        hr = MFSetAttributeSize(
            pStreamMediaType,           // attribute store on which to set the value
            MF_MT_FRAME_SIZE,           // value ID GUID
            m_Width, m_Height);                 // frame width and height
        BREAK_ON_FAIL(hr);

        // Set the frame rate to 30/1.001 - the standard frame rate of NTSC television - as 
        // a 64-bit packed value consisting of a fraction of two integers
        hr = MFSetAttributeRatio(
            pStreamMediaType,           // attribute store on which to set the value
            MF_MT_FRAME_RATE,           // value
            m_VideoFPS, 1);               // frame rate ratio
        BREAK_ON_FAIL(hr);

        // set the average bitrate of the video in bits per second - in this case 10 Mbps
        hr = pStreamMediaType->SetUINT32(MF_MT_AVG_BITRATE, m_VideoBitrate);
        BREAK_ON_FAIL(hr);

        // set the interlace mode to progressive
        hr = pStreamMediaType->SetUINT32(MF_MT_INTERLACE_MODE,
            MFVideoInterlace_Progressive);
        BREAK_ON_FAIL(hr);

        // set the pixel aspect ratio to 1x1 - square pixels
        hr = MFSetAttributeSize(pStreamMediaType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
        BREAK_ON_FAIL(hr);
    } while (false);

    return hr;
}
```

GetTranscodeMediaType() calls either GetTranscodeAudioType() or GetTranscodeVideoType() to do its work.
```Cpp
//
// Set the target target audio and video media types to hard-coded values.  In this case you
// are setting audio to AAC, and video to 720p H.264
//
HRESULT GetTranscodeMediaType(
    CComPtr<IMFMediaType>& pStreamMediaType)
{
    HRESULT hr = S_OK;
    GUID streamMajorType;

    do
    {
        // extract the major type of the source stream from the media type
        hr = pStreamMediaType->GetMajorType(&streamMajorType);
        BREAK_ON_FAIL(hr);

        // if this is an audio stream, configure a hard-coded AAC profile.  If this is a
        // video stream, configure an H.264 profile
        if (streamMajorType == MFMediaType_Audio)
        {
            hr = GetTranscodeAudioType(pStreamMediaType);
        }
        else if (streamMajorType == MFMediaType_Video)
        {
            hr = GetTranscodeVideoType(pStreamMediaType);
        }
    } while (false);

    return hr;
}
```

WriteVideoFrame() copy m_pImage to pData. And set buffer length. Then it create a sample to attach m_pBuffer. Of course, sample must be set with its sampling time and duration. Finally, send the sample to the SinkWriter.

```Cpp
HRESULT WriteVideoFrame(
    DWORD streamIndex,
    const LONGLONG& rtStart
)
{
    IMFSample *pSample = NULL;

    BYTE *pData = NULL;
    // Lock the buffer and copy the video frame to the buffer.
    HRESULT hr = m_pBuffer->Lock(&pData, NULL, NULL);
    if (SUCCEEDED(hr))
    {
        hr = MFCopyImage(
            pData,              // Destination buffer.
            m_cbWidth,          // Destination stride.
            (BYTE*)m_pImage,    // First row in source image.
            m_cbWidth,          // Source stride.
            m_cbWidth,          // Image width in bytes.
            m_Height            // Image height in pixels.
        );
    }
    if (m_pBuffer)
    {
        m_pBuffer->Unlock();
    }

    // Set the data length of the buffer.
    if (SUCCEEDED(hr))
    {
        hr = m_pBuffer->SetCurrentLength(m_cbBuffer);
    }

    // Create a media sample and add the buffer to the sample.
    if (SUCCEEDED(hr))
    {
        hr = MFCreateSample(&pSample);
    }
    if (SUCCEEDED(hr))
    {
        hr = pSample->AddBuffer(m_pBuffer);
    }

    // Set the time stamp and the duration.
    if (SUCCEEDED(hr))
    {
        hr = pSample->SetSampleTime(rtStart);
    }
    if (SUCCEEDED(hr))
    {
        hr = pSample->SetSampleDuration(m_FrameDuration);
    }

    // Send the sample to the Sink Writer.
    if (SUCCEEDED(hr))
    {
        hr = m_pSinkWriter->WriteSample(streamIndex, pSample);
    }

    SafeRelease(&pSample);
    return hr;
}
```

WriteAudioFrame() read sample from m_pSourceReader and send it to SinkWriter.

```Cpp
HRESULT WriteAudioFrame(DWORD streamIndex, LONGLONG& timestamp)
{
    HRESULT hr = S_OK;
    DWORD flags = 0;
    CComPtr<IMFSample> pSample;

    do
    {
        // pull a sample out of the source reader
        hr = m_pSourceReader->ReadSample(
            (DWORD)MF_SOURCE_READER_ANY_STREAM,     // get a sample from any stream
            0,                                      // no source reader controller flags
            &streamIndex,                         // get index of the stream
            &flags,                               // get flags for this sample
            &timestamp,                           // get the timestamp for this sample
            &pSample);                             // get the actual sample
        BREAK_ON_FAIL(hr);

        // The sample can be null if you've reached the end of stream or encountered a
        // data gap (AKA a stream tick).  If you got a sample, send it on.  Otherwise,
        // if you got a stream gap, send information about it to the sink.
        if (pSample != NULL)
        {
            // push the sample to the sink writer
            hr = m_pSinkWriter->WriteSample(streamIndex, pSample);
            BREAK_ON_FAIL(hr);
        }
        else if (flags & MF_SOURCE_READERF_STREAMTICK)
        {
            // signal a stream tick
            hr = m_pSinkWriter->SendStreamTick(streamIndex, timestamp);
            BREAK_ON_FAIL(hr);
        }

        // if a stream reached the end, notify the sink, and increment the number of
        // finished streams
        if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
        {
            hr = m_pSinkWriter->NotifyEndOfSegment(streamIndex);
            BREAK_ON_FAIL(hr);
        }
        // release sample
        pSample = NULL;
    } while (false);

    return hr;
}
```

Process() is a long running function which call InitializeWriter and then enter into a infinite loop. At the start of loop, it signal m_evtRequest for frame and then wait for OpenGL thread to reply with m_evtVideoEnded or m_evtReply. If it is m_evtReply that is signalled, then WriteVideoFrame and/or WriteAudioFrame are called to send sample to SinkWriter. At the end, Finalize() is called to finalize video file.

```Cpp
const bool Process()
{
    if (IsValid())
    {
        DWORD video_stream = 0;
        DWORD audio_stream = 0;

        HRESULT hr = InitializeWriter(&video_stream, &audio_stream);
        if (SUCCEEDED(hr))
        {
            // Send frames to the sink writer.
            LONGLONG rtStart = 0;

            bool success = true;

            HRESULT hr = S_OK;

            bool audio_done = false;

            DWORD audio_stream = 0;
            LONGLONG audio_timestamp = 0;

            while (true)
            {
                SetEvent(m_evtRequest);

                success = true;
                HANDLE arr[2];
                arr[0] = m_evtVideoEnded;
                arr[1] = m_evtReply;
                DWORD dw = WaitForMultipleObjects(2, arr, FALSE, INFINITE);

                if (WAIT_OBJECT_0 == dw)
                {
                    OutputDebugStringA("VideoEnded");
                    break;
                }
                if (WAIT_OBJECT_0 + 1 != dw)
                {
                    OutputDebugStringA("E_FAIL");
                    success = false;
                    break;
                }

                if (success)
                {
                    hr = WriteVideoFrame(video_stream, rtStart);
                    if (FAILED(hr))
                    {
                        success = false;
                        break;
                    }
                    rtStart += m_FrameDuration;

                    if (m_MP3Filename.empty() == false)
                    {
                        if (rtStart < audio_timestamp)
                            continue;

                        if (!audio_done)
                        {
                            hr = WriteAudioFrame(audio_stream, audio_timestamp);
                            if (FAILED(hr))
                            {
                                audio_done = true;
                            }
                        }
                    }
                }
            }

            if (success)
            {
                hr = m_pSinkWriter->Finalize();
            }
            m_pSinkWriter.Release();
            return success;
        }
    }
    return false;
}
```

## Running as asm.js on web browser

This section focus mainly on asm.js aspect of OpenGL framework that comes bundled with the demo. If you are only interested in the video encoder, you can safely ignore this section. If you want to run your app on the web browser with the framework, this is the section for you. The design choice of framework is based on lowest technologies mainstream WebGL and web browser environment can work with. Since WebGL is a safe subset based on OpenGL 2.0 ES in such a way that WebGL calls can be translated to OpenGL 2.0 calls with little effort. Without coincidence, this is also the maximum OpenGL version the framework can support.

**Change your URL**

Change your IP address/domain/port accordingly in SRC_FOLDER in the Program.cpp. SRC_FOLDER is used to download your assets from. Switch to http if you are not using https.

```Cpp
#ifdef __EMSCRIPTEN__
	#define SRC_FOLDER "https://localhost:44319/"
#else
	#define SRC_FOLDER ".\\"
#endif
```

**Recompile the code as asm.js**

Recompile the code for new URL to take effect, if you are on Windows 10 and is comfortable tinkering with Bash commands, [enable Windows Subsystem for Linux](https://www.laptopmag.com/articles/use-bash-shell-windows-10) and install Ubuntu from MS Store and [install Emscripten](https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html). Run GNU make

In the Makefile, change to -s WASM=0

```
make all
```

**Recompile the code as Webassembly**

Remove or change to -s WASM=1 because default option is compiling to Webassembly when WASM is not specified. The reason I did not do so, because IIS do not recognise the wasm mime type added to it.

**No Assimp support**

[Assimp](http://www.assimp.org/) is not supported simply because there is no [Emscripten port](https://github.com/emscripten-ports) for Assimp. Instead, [Tiny OBJ Loader](https://github.com/syoyo/tinyobjloader) is used to load simple 3D model in Wavefront OBJ format. OBJ file extensions ends in *.obj and *.mtl and I have modified the library to load in *.obj.txt and *.mtl.txt because I want to avoid adding new mime types in the web server.

**Downloading your assets**

For drawing anything on screen, derive your class from DrawableSceneComponent and **must** call DownloadFiles() in the constructor to download your assets from relative subfolders because asm.js must have all assets downloaded before the main rendering loop is run. DownloadFiles calls OpenFile on the files directly on a desktop app. If the code is compiled with EMSCRIPTEN macro defined, it will first download files and then open.

**No downloading of shader**

The shader code are inline with C++11 raw string literals to save downloading effort.

## Hardware Acceleration

Hardware acceleration is available now by adding these 3 lines to set MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS in attributes and pass it to MFCreateSinkWriterFromURL(). The HEVC quality problem mentioned earlier is fixed with h/w acceleration.

```
CComPtr<IMFAttributes> attrs;
MFCreateAttributes(&attrs, 1);
attrs->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);

hr = MFCreateSinkWriterFromURL(m_DestFilename.c_str(), nullptr, attrs, &m_pSinkWriter);
```

When you run video encoder without OpenGL, you should see "GPU 1 - Video Encode" on Windows 10 task manager. With OpenGL, you see "GPU 1 - Copy"

![Image of TaskMgr](https://github.com/shaovoon/video_encoder_for_ogl_dx/blob/master/images/TaskMgr.png)

## Quality parameters in Constructor in v0.4.2

* int numWorkerThreads: 0 leaves to default
* int qualityVsSpeed: [0:100] 0 for speed, 100 for quality
* RateControlMode mode: 3 modes to choose from UnconstrainedVBR, Quality, CBR (VBR is variable bitrate and CBR is constant bitrate)
* int quality: Only valid when mode is Quality. [0:100] 0 for smaller file size and lower quality, 100 for bigger file size and higher quality

## Reference Book

[Developing Microsoft Media Foundation Applications](https://www.amazon.com/gp/product/0735656592/ref=as_li_qf_asin_il_tl?ie=UTF8&tag=qhyti98po-20&creative=9325&linkCode=as2&creativeASIN=0735656592&linkId=3a5b32bf135fdbb0772831efc7e6c96f) by Microsoft Press
