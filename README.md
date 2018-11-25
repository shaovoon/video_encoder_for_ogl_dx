# H264 Video Encoder for OpenGL and DirectX

Writing documentation in process. Do not read or clone this repo now.

### Introduction

I worked on this video encoder while writing my Windows Store App, Mandy Frenzy, a photo slideshow app for ladies. Right now, I am feeling burnt out so I am taking a short hiatus. Meanwhile I write a series of short articles as a way to document this app. This video encoder is header file only (H264Writer.h), based on Microsoft Media Foundation, not the old DirectShow and it is tested on Windows 10. However, it should work fine on Windows 7/8 as well.

The documentation is divided into 3 main sections. First section is for those reader who like to see the demo up and running and on how to modify the parameters. Second section is on how to integrate it with your OpenGL framework. The demo uses a renderer framework used in Paul Varcholik's OpenGL Essentials LiveLessons. A tutorial on how to integrate with DirectX comes later. In theory, this video encoder should integrate well with other graphics API like Vulkan, afterall, all it needs to be supplied with a video buffer and some synchronization in tandem to perform its work. Third section is on the explanation of the internals of the video encoder.

### Running the Demo

All the required libraries are included in the repository. The required dlls are copied automatically to the Release or Debug folder for Win32 post builds. x64 build is unbuildable due to inability to find a x64 zlib lib/dll on the web; this is a linking problem lies with the OpenGL renderer, not video encoder.

To see the OpenGL demo, open up SDL_App.sln in Visual Studio and build the SDL_App project

To run the video encoding demo, open up H264SinkWriter.cpp and in the main function, modify the configFile, musicFile and videoFile to the paths on your machine. configFile is found in the $(SolutionDir)SDL_App folder. musicFile should be a mp3 file and if it is left blank, the final video shall not have music. videoFile is the encoded video.

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
Version=1.0.0.0
ProjectName=My First Project
ScreenWidth=800
ScreenHeight=600
LogPath=C:\Users\shaov\Documents\log.txt
FPS=60
```

Now the demo does not handle aspect ratio and it always stick with 4:3 ratio. If you enter anything which is 16:9, or wider than 4:3, in screen width and height, your video will look stretched. FPS entry is for the integer number of frames per second; there is no way to enter a decimal number like 29.7777. Version and ProjectName is unimportant, you can ignore them.

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
    if(elapsed_time > 5.0f) // During video encoding, only run for 5 seconds.
    {
        Scene::setVideoEnded();
    }
#endif
}
```

To modify the bitrate, there is no good way to do it except to edit the m_VideoBitrate manually in the H264Writer constructor.

```Cpp
// H264Writer constructor
H264Writer(const wchar_t* mp3_file, const wchar_t* src_file, const wchar_t* dest_file, VideoCodec codec) :
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
    m_VideoBitrate(4000000),
    m_EncCommonQuality(100),
    m_VideoCodec(codec),
    m_nStreams(0)
{...}

```

### Integration with your OpenGL Framework

### How the code works