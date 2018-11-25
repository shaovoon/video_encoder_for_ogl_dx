// H264SinkWriter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../SDL_App_DLL/Program.h"
#include "H264Writer.h"

int check_config_file(const wchar_t* file, int* width, int* height, int* fps)
{
	return ::check_project_file(file, width, height, fps);
}
int encoder_start(UINT** pixels, HANDLE evtRequest, HANDLE evtReply, HANDLE evtExit, HANDLE evtVideoEnded, const WCHAR* szUrl)
{
	return ::encoder_main(pixels, evtRequest, evtReply, evtExit, evtVideoEnded, szUrl);
}

int main()
{
	/*
	std::vector<std::wstring> encoders;
	if (H264Writer::EnumVideoEncoder(encoders, true, false))
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
	*/
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
	return 1;
}
