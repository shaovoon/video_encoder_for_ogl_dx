/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#include "ImageDownloader.h"
#include "csv_stream.h"
#include "StringUtils.h"
#include "Utility.h"
#include "DownloadSingleton.h"

extern Library::DownloadSingleton gDownloadSingleton;

namespace Library
{
	RTTI_DEFINITIONS(ImageDownloader)

	ImageDownloader::ImageDownloader(const std::string& folder, const std::vector<ImageDownloader::ImageInfo>& file_list)
		: mFolder(folder)
		, mImageInfoList(file_list)
		, mCanOpenFiles(0)
	{
		DownloadFiles();
	}

	ImageDownloader::~ImageDownloader()
	{
		for (size_t i = 0; i < mImageInfoList.size(); ++i)
		{
			ImageInfo& info = mImageInfoList[i];

			if (info.mpTexture != nullptr)
			{
				delete info.mpTexture;
				info.mpTexture = nullptr;
			}
		}
	}

	void ImageDownloader::Initialize() 
	{
		CreateTextures();

		SetInitialized(true);
	}

	void ImageDownloader::DownloadFiles()
	{
		std::string srcFolder = Utility::CombineFolder(gDownloadSingleton.getSrcFolder(), mFolder);

		for (size_t i = 0; i < mImageInfoList.size(); ++i)
		{
			printf("mImageInfoList[i].mFile:%s", mImageInfoList[i].mFile.c_str());
			gDownloadSingleton.DownloadFile(this, srcFolder, mImageInfoList[i].mFile,
				Library::DownloadableComponent::FileType::IMAGE_FILE);
		}
	}

	bool ImageDownloader::OpenFile(const char* file, FileType file_type)
	{
		if (file_type == Library::DownloadableComponent::FileType::IMAGE_FILE)
		{
			if (Utility::FileExists(file))
				++mCanOpenFiles;
			return true;
		}

		return false;
	}

	bool ImageDownloader::CreateTextures()
	{
		if (mImageInfoList.size() == 0)
			return false;

		for (size_t i = 0; i < mImageInfoList.size(); ++i)
		{
			Texture* ptexture = new Texture();
			ImageInfo& info = mImageInfoList[i];
			std::string folder = mFolder;
#ifdef __EMSCRIPTEN__
			folder = "";
#endif
			if (info.mDim == info.mWidth&& info.mWidth == info.mHeight)
			{
				ptexture->LoadImageFile(folder,
					info.mFile, false, false);
			}
			else
			{
				ptexture->LoadSquareImageFile(folder,
					info.mFile, info.mDim,
					info.mWidth, info.mHeight, false);
			}

			info.mpTexture = ptexture;
		}

		return true;
	}

}
