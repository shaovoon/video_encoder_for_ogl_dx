/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#include "Utility.h"
#include <algorithm>
#include <exception>
#include <cctype>
#include <cstdio>
#include "zlib.h"

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include <Shlwapi.h>
	#include <cstdlib>
#endif // _WIN32

namespace Library
{
	std::string Utility::CurrentDirectory()
	{
#ifdef _WIN32
		WCHAR buffer[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, buffer);
		std::wstring currentDirectoryW(buffer);

		return std::string(currentDirectoryW.begin(), currentDirectoryW.end());
#else
		return "";
#endif
	}

	std::wstring Utility::ExecutableDirectory()
	{
#ifdef _WIN32
		WCHAR buffer[MAX_PATH];
		GetModuleFileName(nullptr, buffer, MAX_PATH);
		PathRemoveFileSpec(buffer);

		return std::wstring(buffer);
#else
		return L"";
#endif

	}

	void Utility::GetFileName(const std::string& inputPath, std::string& filename)
	{
		std::string fullPath(inputPath);
		std::replace(fullPath.begin(),fullPath.end(),'\\','/');

		std::string::size_type lastSlashIndex = fullPath.find_last_of('/');

		if (lastSlashIndex == std::string::npos)
		{
			filename = fullPath;
		}
		else
		{
			filename = fullPath.substr(lastSlashIndex + 1, fullPath.size() - lastSlashIndex- 1);
		}
	}

	void Utility::GetDirectory(const std::string& inputPath, std::string& directory)
	{
		std::string fullPath(inputPath);
		std::replace(fullPath.begin(),fullPath.end(),'\\','/');

		std::string::size_type lastSlashIndex = fullPath.find_last_of('/');

		if (lastSlashIndex == std::string::npos)
		{
			directory = "";
		}
		else
		{		
			directory = fullPath.substr(0, lastSlashIndex);
		}
	}

	void Utility::GetFileNameAndDirectory(const std::string& inputPath, std::string& directory, std::string& filename)
	{
		std::string fullPath(inputPath);
		std::replace(fullPath.begin(),fullPath.end(),'\\','/');

		std::string::size_type lastSlashIndex = fullPath.find_last_of('/');

		if (lastSlashIndex == std::string::npos)
		{
			directory = "";
			filename = fullPath;
		}
		else
		{
			directory = fullPath.substr(0, lastSlashIndex);
			filename = fullPath.substr(lastSlashIndex + 1, fullPath.size() - lastSlashIndex- 1);
		}
	}
	
	void Utility::LoadBinaryFile(const std::string& filename, std::vector<char>& data)
	{
#ifdef _MSC_VER
		FILE *f = nullptr;
		fopen_s(&f, filename.c_str(), "rb");
#else
		FILE *f = fopen(filename.c_str(), "rb");
#endif
		if (!f)
		{
			char buf[1000];
			SPRINTF(buf, "Utility::LoadBinaryFile: Could not open file: %s", filename.c_str());
			throw std::runtime_error(buf);
		}
		fseek(f, 0, SEEK_END);
		long fsize = ftell(f);
		fseek(f, 0, SEEK_SET);  //same as rewind(f);

		data.resize(fsize + 1);
		fread(data.data(), fsize, 1, f);
		fclose(f);

		data[fsize] = 0;
	}

	void Utility::LoadShaderFile(const std::string& filename, std::string& data, Precision precision)
	{
#ifndef _WIN32
		// insert the precision in the first line of the shader
		if (precision == Precision::highp)
			data += "precision highp float;\n";
		else if (precision == Precision::mediump)
			data += "precision mediump float;\n";
		else
			data += "precision lowp float;\n";
#endif

#ifdef _MSC_VER
		FILE *f = nullptr;
		fopen_s(&f, filename.c_str(), "rt");
#else
		FILE *f = fopen(filename.c_str(), "rt");
#endif
		if (!f)
		{
#ifndef _WIN32
			data = "";
#endif
			char buf[1000];
			SPRINTF(buf, "Utility::LoadShaderFile: Could not open file: %s", filename.c_str());
			throw std::runtime_error(buf);
		}
		fseek(f, 0, SEEK_END);
		long fsize = ftell(f);
		fseek(f, 0, SEEK_SET);  //same as rewind(f);

		data.resize(fsize + 1);
		fread(&data[0], fsize, 1, f);
		fclose(f);

		data[fsize] = 0;
	}


	void Utility::ToWideString(const std::string& source, std::wstring& dest)
	{
		dest.assign(source.begin(), source.end());
	}

	std::wstring Utility::ToWideString(const std::string& source)
	{
		std::wstring dest;
		dest.assign(source.begin(), source.end());

		return dest;
	}

	void Utility::ToString(const std::wstring& source, std::string& dest)
	{
		dest.assign(source.begin(), source.end());
	}

	std::string Utility::ToString(const std::wstring& source)
	{
		std::string dest;
		dest.assign(source.begin(), source.end());

		return dest;
	}

	void Utility::PathJoin(std::wstring& dest, const std::wstring& sourceDirectory, const std::wstring& sourceFile)
	{
#ifdef _WIN32
		WCHAR buffer[MAX_PATH];

		PathCombine(buffer, sourceDirectory.c_str(), sourceFile.c_str());
		dest = buffer;
#endif
	}

	std::wstring Utility::GetFileExtension(const std::wstring& source)
	{
		std::wstring ext;
#ifdef _WIN32
		ext = PathFindExtensionW(source.c_str());
#else
		size_t pos = source.find_last_of(L'.');
		if (pos != std::string::npos)
		{
			ext = source.substr(pos);
		}
#endif
		return ext;
	}

	std::string Utility::GetFileExtension(const std::string& source)
	{
		std::string ext;
#ifdef _WIN32
		ext = PathFindExtensionA(source.c_str());
#else
	size_t pos = source.find_last_of('.');
	if (pos != std::string::npos)
	{
		ext = source.substr(pos);
	}
#endif
		return ext;
	}

	// cross platform
	std::string Utility::CombineFolder(const std::string& f1, const std::string& f2)
	{
#ifdef __EMSCRIPTEN__
		const char sep = '/';
#else
		const char sep = '\\';
#endif
		std::string dest = f1;
		if (f1.size() > 0 && f1[f1.size()-1]!=sep)
		{
			dest += sep;
		}
		dest += f2;
		if (f2.size() > 0 && f2[f2.size() - 1] != sep)
		{
			dest += sep;
		}
		return dest;
	}

	// cross platform
	std::string Utility::CombineFilePath(const std::string& f1, const std::string& f2)
	{
#ifdef __EMSCRIPTEN__
		const char sep = '/';
#else
		const char sep = '\\';
#endif
		std::string dest = f1;
		if (f1.size() > 0 && f1[f1.size() - 1] != sep)
		{
			dest += sep;
		}
		dest += f2;
		return dest;
	}

	std::string Utility::ToUpperString(const std::string &str)
	{
		std::string dest = "";
		for (size_t i = 0; i < str.size(); ++i)
		{
			dest += std::toupper(str[i]);
		}
		return dest;
	}
#ifdef _MSC_VER
	DWORD Utility::RunSilent(const char* strFunct, const char* strstrParams)
	{
		STARTUPINFOA StartupInfo;
		PROCESS_INFORMATION ProcessInfo;
		char Args[4096];
		char *pEnvCMD = NULL;
		char *pDefaultCMD = "CMD.EXE";
		ULONG rc;

		memset(&StartupInfo, 0, sizeof(StartupInfo));
		StartupInfo.cb = sizeof(STARTUPINFO);
		StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
		StartupInfo.wShowWindow = SW_HIDE;

		Args[0] = 0;

		size_t requiredSize;

		getenv_s(&requiredSize, NULL, 0, "COMSPEC");

		pEnvCMD = (char*) malloc(requiredSize * sizeof(char));
		if (!pEnvCMD)
		{
			return 1;
		}

		getenv_s(&requiredSize, pEnvCMD, requiredSize, "COMSPEC");

		if (pEnvCMD){

			strcpy_s(Args, pEnvCMD);
		}
		else{
			strcpy_s(Args, pDefaultCMD);
		}

		// "/c" option - Do the command then terminate the command window
		strcat_s(Args, " /c ");
		//the application you would like to run from the command window
		strcat_s(Args, strFunct);
		strcat_s(Args, " ");
		//the parameters passed to the application being run from the command window.
		strcat_s(Args, strstrParams);

		if (!CreateProcessA(NULL, Args, NULL, NULL, FALSE,
			CREATE_NEW_CONSOLE,
			NULL,
			NULL,
			&StartupInfo,
			&ProcessInfo))
		{
			return GetLastError();
		}

		WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
		if (!GetExitCodeProcess(ProcessInfo.hProcess, &rc))
			rc = 0;

		CloseHandle(ProcessInfo.hThread);
		CloseHandle(ProcessInfo.hProcess);

		return rc;
	}
#endif // _MSC_VER

	unsigned long Utility::GetFileSize(const char *filename)
	{
		FILE *pFile = NULL;

#ifdef _MSC_VER
		fopen_s(&pFile, filename, "rb");
#else
		pFile = fopen(filename, "rb");
#endif
		if (!pFile)
			return 0;

		fseek(pFile, 0, SEEK_END);
		unsigned long size = ftell(pFile);
		fclose(pFile);
		return size;
	}

	bool Utility::DecompressFile(const char *infilename, const char *outfilename)
	{
		using namespace std;
		gzFile infile = gzopen(infilename, "rb");
		FILE *outfile = NULL;

#ifdef _MSC_VER
		fopen_s(&outfile, outfilename, "wb");
#else
		outfile = fopen(outfilename, "wb");
#endif

		if (!infile || !outfile) return false;

		char buffer[128];
		int num_read = 0;
		while ((num_read = gzread(infile, buffer, sizeof(buffer))) > 0) {
			fwrite(buffer, 1, num_read, outfile);
		}

		gzclose(infile);
		fclose(outfile);

		return true;
	}

	bool Utility::CompressFile(const char *infilename, const char *outfilename)
	{
		using namespace std;
		FILE *infile = NULL;
#ifdef _MSC_VER
		fopen_s(&infile, infilename, "rb");
#else
		infile = fopen(infilename, "rb");
#endif

		gzFile outfile = gzopen(outfilename, "wb");
		if (!infile || !outfile) return false;

		char inbuffer[128];
		int num_read = 0;
		unsigned long total_read = 0;
		while ((num_read = fread(inbuffer, 1, sizeof(inbuffer), infile)) > 0) {
			total_read += num_read;
			gzwrite(outfile, inbuffer, num_read);
		}
		fclose(infile);
		gzclose(outfile);

		printf("Read %ld bytes, Wrote %ld bytes, Compression factor %4.2f%%\n",
			total_read, GetFileSize(outfilename),
			(1.0 - GetFileSize(outfilename)*1.0 / total_read)*100.0);

		return true;
	}

	bool Utility::FileExists(const char *fname)
	{
		FILE *file = NULL;
#ifdef _MSC_VER
		fopen_s(&file, fname, "r");
#else
		file = fopen(fname, "r");
#endif
		if (file)
		{
			fclose(file);
			return true;
		}
		return false;
	}
}