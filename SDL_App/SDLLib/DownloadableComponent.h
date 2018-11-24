/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#pragma once
#include "Common.h"
#include <string>
#include <map>

namespace Library
{
	class DownloadableComponent : public RTTI
	{
		RTTI_DECLARATIONS(DownloadableComponent, RTTI)

	public:
		DownloadableComponent();
		~DownloadableComponent();

		enum class FileType
		{
			INI_FILE,
			CSV_FILE,
			SRT_FILE,
			VERTEX_SHADER,
			FRAGMENT_SHADER,
			IMAGE_FILE_POW_OF_2,
			SPECULAR_IMAGE_FILE_POW_OF_2,
			IMAGE_FILE,
			IMAGE_FILE_2,
			SHADOW_IMAGE_FILE_POW_OF_2,
			SHADOW_IMAGE_FILE,
			BKGD_IMAGE,
			OBJ_FILE,
			OBJ_GZ_FILE,
			ALL_OBJ_GZ_FILE,
			MTL_FILE,
			GZ_FILE,
			MESH
		};

		virtual void DownloadFiles() = 0;

		void AddFile(const std::string& file, FileType file_type);

		bool LoadFile(const char* file);
		virtual bool OpenFile(const char* file, FileType file_type) = 0;

		bool DownloadReady() { return mPending == mCompleted; }

		bool IsInitialized() { return mInitialized; }
		void SetInitialized(bool initialized) { mInitialized = initialized; }

	private:
		int mPending;
		int mCompleted;
		bool mInitialized;
		std::map<std::string, FileType> mFileTypeMap;
	};

}