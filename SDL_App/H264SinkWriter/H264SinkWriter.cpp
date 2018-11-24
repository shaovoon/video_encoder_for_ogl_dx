// H264SinkWriter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "H264Writer.h"

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
	std::wstring srcFilename(L"D:\\GitHub\\video_encoder_for_ogl_dx\\SDL_App\\SDL_App\\config.txt");
	std::wstring mp3File(L"D:\\FMA.mp3");

	H264Writer writer(mp3File.c_str(), srcFilename.c_str(), L"C:\\Users\\shaov\\Documents\\video.mp4", VideoCodec::H264);
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
