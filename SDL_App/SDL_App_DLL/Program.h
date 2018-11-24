// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SDL_APP_DLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SDL_APP_DLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SDL_APP_DLL_EXPORTS
#define SDL_APP_DLL_API __declspec(dllexport)
#else
#define SDL_APP_DLL_API __declspec(dllimport)
#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

SDL_APP_DLL_API int WINAPI check_project_file(const wchar_t* file, int* width, int* height, int* fps);
SDL_APP_DLL_API int WINAPI encoder_main(UINT** pixels, HANDLE evtRequest, HANDLE evtReply, HANDLE evtExit, HANDLE evtVideoEnded, const WCHAR* szUrl);
