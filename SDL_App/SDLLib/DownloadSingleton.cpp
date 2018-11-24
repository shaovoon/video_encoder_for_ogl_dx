/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "DownloadSingleton.h"
#include <VariaNetDebugPrint.h>
#include <constants.h>

#ifdef __EMSCRIPTEN__
	#include <emscripten.h>
#endif

extern Logger gLogger;

namespace Library
{

	DownloadSingleton::DownloadSingleton(const std::string& src_folder)
		: mSrcFolder(src_folder)
	{
	}

	DownloadSingleton::~DownloadSingleton()
	{
	}

	bool DownloadSingleton::DownloadFile(DownloadableComponent* comp, const std::string& folder,
		const std::string& file_name, DownloadableComponent::FileType file_type, bool deferred)
	{
		if (file_name.empty())
			return true;
		//printf("%s%s to be downloaded\n", folder.c_str(), file_name.c_str());
		if (mFileMap.find(file_name) != mFileMap.end())
		{
			printf("error:file already exists:%s, folder:%s\n", file_name.c_str(), folder.c_str());
		}
		std::string full_path = folder + file_name;
#ifdef __EMSCRIPTEN__
		comp->AddFile(file_name, file_type);
		mFileMap[file_name] = comp;
		if(deferred==false)
		{
			emscripten_async_wget(full_path.c_str(), file_name.c_str(), file_callback, error_callback);
		}
		else
		{
			m_FullPathList.push_back(std::make_pair(full_path, file_name));
		}
#else
		comp->AddFile(full_path, file_type);
		mFileMap[full_path] = comp;
		file_callback(full_path.c_str());
#endif
		return true;
	}

#ifdef __EMSCRIPTEN__
	void DownloadSingleton::DownloadAllNow()
	{
		FullPathListType::const_iterator it = m_FullPathList.begin();
		for (; it != m_FullPathList.end(); ++it)
		{
			emscripten_async_wget(it->first.c_str(), it->second.c_str(), file_callback, error_callback);
		}
		m_FullPathList.clear();
	}
	bool DownloadSingleton::IsAllDownloaded()
	{
		std::map<std::string, DownloadableComponent*>::iterator it = mFileMap.begin();
		for (; it != mFileMap.end(); ++it)
		{
			if (it->second->DownloadReady()==false)
				return false;
		}
		return true;
	}

#endif

	bool DownloadSingleton::ReceiveFile(const char * file)
	{
		printf("Receive file:%s\n", file);
		//std::string file_path = file;
		std::map<std::string, DownloadableComponent*>::iterator it = mFileMap.begin();
		for (; it != mFileMap.end(); ++it)
		{
			if (strcmp(it->first.c_str(), file) == 0)
				break;
		}
		if (it == mFileMap.end())
		{
			printf("File not found in map:%s\n", file);

			char buf[1000];
			SPRINTF(buf, "DownloadSingleton::ReceiveFile error : file not found in map : %s", file);

			gLogger.Log(buf);
			return false;
		}

		DownloadableComponent* comp = it->second;
		return comp->LoadFile(file);
	}

	void DownloadSingleton::FileError(const char * file)
	{
		char buf[1000];
		SPRINTF(buf, "DownloadSingleton::FileError error: file: %s", file);

		gLogger.Log(buf);
	}

}

extern Library::DownloadSingleton gDownloadSingleton;

void file_callback(const char * file)
{
	if(file[0] == '/')
		++file;
	gDownloadSingleton.ReceiveFile(file);
}

void error_callback(const char * file)
{
	if (file[0] == '/')
		++file;
	gDownloadSingleton.FileError(file);
}

