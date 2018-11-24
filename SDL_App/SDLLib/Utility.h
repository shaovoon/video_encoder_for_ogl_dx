/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#pragma once

#include "Common.h"
#include "Enums.h"

namespace Library
{
	class Utility
	{
	public:
		static std::string CurrentDirectory();
		static std::wstring ExecutableDirectory();
		static void GetFileName(const std::string& inputPath, std::string& filename);
		static void GetDirectory(const std::string& inputPath, std::string& directory);
		static void GetFileNameAndDirectory(const std::string& inputPath, std::string& directory, std::string& filename);
		static void LoadBinaryFile(const std::string& filename, std::vector<char>& data);
		static void LoadShaderFile(const std::string& filename, std::string& data, Precision precision=Precision::mediump);
		static void ToWideString(const std::string& source, std::wstring& dest);
		static std::wstring ToWideString(const std::string& source);
		static void ToString(const std::wstring& source, std::string& dest);
		static std::string ToString(const std::wstring& source);
		static void PathJoin(std::wstring& dest, const std::wstring& sourceDirectory, const std::wstring& sourceFile);
		static std::wstring GetFileExtension(const std::wstring& source);
		static std::string GetFileExtension(const std::string& source);
		static std::string CombineFolder(const std::string& f1, const std::string& f2);
		static std::string CombineFilePath(const std::string& f1, const std::string& f2);

		static std::string ToUpperString(const std::string &str);
		template<typename T>
		static T clamp(T val, T min_val, T max_val)
		{
			if (val < min_val)
				return min_val;
			if (val > max_val)
				return max_val;

			return val;
		}

#ifdef _WIN32
		static DWORD RunSilent(const char* strFunct, const char* strstrParams);
#endif // _WIN32

		static unsigned long GetFileSize(const char *filename);
		static bool DecompressFile(const char *infilename, const char *outfilename);
		static bool CompressFile(const char *infilename, const char *outfilename);
		static bool FileExists(const char *fname);

	private:
		Utility();
		Utility(const Utility& rhs);
		Utility& operator=(const Utility& rhs);
	};

	inline void LogFunction(const char* class_name, const char* func_name)
	{
		printf("%s.%s\n", class_name, func_name);
	}

	#define LOGFUNCTION(x,y) LogFunction(x,y)

#ifdef _WIN32
	#define MY_FUNC __FUNCSIG__ 
#else
	#define MY_FUNC __PRETTY_FUNCTION__
#endif


}