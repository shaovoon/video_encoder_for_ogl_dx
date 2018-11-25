# H264 Video Encoder for OpenGL and DirectX

Writing documentation in process. Do not read or clone this repo now.

### Requirements

* Visual C++ 2015/2017
* Windows 7/8/10

### Introduction

I worked on this video encoder while writing my Windows Store App, Mandy Frenzy, a photo slideshow app for ladies. Right now, I am feeling burnt out so I am taking a short hiatus. Meanwhile I write a series of short articles as a way to document this app. This video encoder is header file only (H264Writer.h), based on Microsoft Media Foundation, not the old DirectShow and it is tested on Windows 10. However, it should work fine on Windows 7/8 as well.

What are the pros and cons of this encoder over FFmpeg?

FFmpeg is GPL and so you may not concerned if you just want to encode the output of your personal renderer. For my freemium app, I prefer to steer clear of the licensing issues. How hobbyist usually encode their frames with FFmpeg is to save all the frames in HDD first which limits the number of frames and also directly impacted video duration that can be saved depending on the free HDD space. The extra step of saving and opening the files has negative impact of the encoding speed. Of course, tight integration with FFmpeg code may eliminate the frame saving part. On the other hand, this encoder reads RGB values from the framebuffer provided. The downside is it is not portable and only works on Windows 7/8/10.

The same OpenGL renderer can be compiled into 3 modes: normal OpenGL display mode, Video Encoder mode and Emscripten mode. The latter two's code sections are respectively guarded by VIDEO_ENCODER and __EMSCRIPTEN__ macros. You can, by all means, use your own renderer with the video encoder. The default OpenGL renderer is just provided to show a working demo.

The documentation is divided into 3 main sections. First section is get the demo up and running and on how to modify the parameters. Second section is on how to integrate it with your OpenGL framework. The demo uses a renderer framework used in Paul Varcholik's OpenGL Essentials LiveLessons. A tutorial on how to integrate with DirectX comes later. In theory, this video encoder should integrate well with other graphics API like Vulkan, afterall, all it needs to be supplied with a video buffer and some synchronization in tandem to perform its work. Third section (empty) is on the explanation of the internals of the video encoder. And the last section explains the Emscripten part required to compile into asm.js or Webassembly.

### Running the Demo

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
    if(elapsed_time > 5.0f) // During video encoding, only run for 5 seconds.
    {
        Scene::setVideoEnded();
    }
#endif
}
```

To modify the bitrate to other value, you can set the 5th parameter of the . You have to experiment to find out the optimal bitrate that can encode a good quality video.

```Cpp
// H264Writer constructor
H264Writer(const wchar_t* mp3_file, const wchar_t* src_file, const wchar_t* dest_file, VideoCodec codec, UINT32 bitrate = 4000000) :
{...}
```

To run the demo executable by itself, you need to copy the config.txt, Images and Models folders to the Release/Debug folder. The SDL2 dlls would have already copied during post build.

By default, demo displays a rotating UFO saucer, to display other 3D model, just uncomment the other lines in CreateComponents(). 

```Cpp
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

        //mStarModel = std::unique_ptr<StarModel>(new StarModel(*this, *mCamera, glm::vec3(1.0f,1.0f,0.0f), "Star.obj.txt", "Star.mtl.txt"));
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

### Integration with your OpenGL Framework

The fastest way to find all the encoding related code is to search for VIDEO_ENCODER macro in the source code. 

### How the code works