/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "DownloadableComponent.h"
#include <VariaNetDebugPrint.h>
#include <constants.h>

extern Logger gLogger;
namespace Library
{
	RTTI_DEFINITIONS(DownloadableComponent)

	DownloadableComponent::DownloadableComponent()
		: mPending(0)
		, mCompleted(0)
		, mInitialized(false)
	{
	}

	DownloadableComponent::~DownloadableComponent()
	{
	}

	void DownloadableComponent::AddFile(const std::string& file, FileType file_type)
	{
		++mPending;

		if (mFileTypeMap.find(file) != mFileTypeMap.end())
		{
			char buf[1000];
			SPRINTF(buf, "DownloadableComponent::DownloadFile error: file already exists:%s", file.c_str());
			gLogger.Log(buf);
		}

		mFileTypeMap[file] = file_type;
	}

	bool DownloadableComponent::LoadFile(const char* file)
	{
		std::map<std::string, FileType>::iterator it = mFileTypeMap.begin();
		for (; it != mFileTypeMap.end(); ++it)
		{
			if (strcmp(it->first.c_str(), file) == 0)
				break;
		}
		if (it == mFileTypeMap.end())
		{
			return false;
		}

		bool ret = OpenFile(file, it->second); // have to be implementation defined.

		if (ret) ++mCompleted;

		return ret;
	}
}