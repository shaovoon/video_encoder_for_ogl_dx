/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/

#pragma once
#include <string>
#include <vector>
#include "SceneComponent.h"
#include "Texture.h"

namespace Library
{
	class ImageDownloader : public SceneComponent
	{
		RTTI_DECLARATIONS(ImageDownloader, SceneComponent)

	public:
		struct ImageInfo
		{
			ImageInfo() : mDim(0), mWidth(0), mHeight(0) {}
			std::string mFile;
			int mDim;
			int mWidth;
			int mHeight;
			Texture* mpTexture;
		};

		ImageDownloader(const std::string& folder, const std::vector<ImageInfo>& file_list);
		~ImageDownloader();

		void Initialize() override;
		virtual void DownloadFiles() override;
		virtual bool OpenFile(const char* file, FileType file_type) override;

		bool CreateTextures();

		const std::vector<ImageInfo>& GetImageInfoList() const { return mImageInfoList; }

		const std::string& GetFolder() const { return mFolder; }
		void SetFolder(const std::string& val) { mFolder = val; }

		const std::vector<Texture*>& GetVecTexture() const { return mVecTexture; }
		void SetVecTexture(const std::vector<Texture*>& val) { mVecTexture = val; }

		int CanOpenFileCount() const { return mCanOpenFiles; }

	private:
		void Parse(const std::string& name, const std::string& value);

		std::string mFolder;
		std::vector<ImageInfo> mImageInfoList;
		std::vector<Texture*> mVecTexture;
		int mCanOpenFiles;
	};
}

