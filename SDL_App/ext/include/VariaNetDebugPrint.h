#pragma once

#include <string>
#define __STDC_WANT_LIB_EXT1__
#include <time.h>
#include <ctime>
#include <chrono>
#include <codecvt> 
#include <cstdio>

struct Logger
{
#ifdef _WIN32
	#include <Windows.h>
#endif // WIN32

	static void DebugPrint(const char *s)
	{
	#ifdef _WIN32
		OutputDebugStringA(s);
	#endif // WIN32
	}

	static void DebugPrint(const wchar_t *s)
	{
	#ifdef _WIN32
		OutputDebugStringW(s);
	#endif // WIN32
	}

	void Log(const char *s)
	{
	#ifdef __EMSCRIPTEN__
		printf("%s\n", s);
	#endif

	// Do not log in emscripten, only in win32
	#ifdef _WIN32
		if (mLogFile.empty())
			mLogFile = "c:\\temp\\log.txt"; // TODO : fix this static member bug!
		FILE* f = nullptr;
		fopen_s(&f, mLogFile.c_str(), "a+");
		if (f)
		{
			SYSTEMTIME st;
			::GetSystemTime(&st);

			fprintf( f, "%hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu : %s\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, s);
			fflush(f);
			fclose(f);
		}
	#endif
	}

#ifdef _MSC_VER
	static bool FileExists(const std::wstring& name) 
	{
		FILE *file = NULL;
		_wfopen_s(&file, name.c_str(), L"r");
		if (file) {
			std::fclose(file);
			return true;
		}
		else {
			std::fclose(file);
			return false;
		}
	}

	static bool WriteBOM(const std::wstring& name)
	{
		FILE *file = NULL;
		_wfopen_s(&file, name.c_str(), L"wb");

		unsigned char bom[3] = { 0xEF, 0xBB, 0xBF };

		fwrite(bom, 1, 3, file);

		std::fclose(file);

		return true;
	}
	void setLogFile(const std::string& file)
	{
		mLogFile = file;
	}
private:
	std::string mLogFile;
#endif
};