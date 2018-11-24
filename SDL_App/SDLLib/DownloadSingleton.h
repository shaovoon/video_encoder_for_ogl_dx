/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#pragma once

#include <string>
#include <map>
#include "DownloadableComponent.h"

void file_callback(const char * file);
void error_callback(const char * file);

namespace Library
{

	class DownloadSingleton
	{
	public:
		DownloadSingleton(const std::string& src_folder);
		~DownloadSingleton();

		// folder is http url in Emscripten
		bool DownloadFile(DownloadableComponent* comp, const std::string& folder, const std::string& file_name, 
			DownloadableComponent::FileType file_type, bool deferred=true);
		bool ReceiveFile(const char * file);
		void FileError(const char * file);

		const std::string& getSrcFolder() const { return mSrcFolder; }
		void setSrcFolder(const std::string& val) { mSrcFolder = val; }
#ifdef __EMSCRIPTEN__
		void DownloadAllNow();
		bool IsAllDownloaded();
#endif
	private:
		std::map<std::string, DownloadableComponent*> mFileMap;
		std::string mSrcFolder;
#ifdef __EMSCRIPTEN__
		typedef std::vector< std::pair<std::string, std::string> > FullPathListType;
		FullPathListType m_FullPathList;
#endif
	};

}