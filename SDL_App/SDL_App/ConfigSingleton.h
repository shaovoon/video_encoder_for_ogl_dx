#pragma once
#include <string>
#include <DownloadableComponent.h>
#include <ImageDownloader.h>

using namespace Library;

class ConfigSingleton : public DownloadableComponent
{
	RTTI_DECLARATIONS(ConfigSingleton, DownloadableComponent)

public:
	ConfigSingleton();
	~ConfigSingleton();

	virtual void DownloadFiles() override;
	virtual bool OpenFile(const char* file, FileType file_type) override;

	bool IsLoadSuccess() const { return m_LoadSuccess; }

	bool LoadConfigFile(const std::string& file);
	bool WriteConfigFile(const std::string& file);

	const std::string& getVersion() const { return m_Version; }
	const std::string& getProjectName() const { return m_ProjectName; }
	int GetScreenWidth() const { return m_ScreenWidth; }
	int GetScreenHeight() const { return m_ScreenHeight; }
	const std::string& getLogPath() const { return m_LogPath; }

	const std::string& getBubblyText() const { return m_BubblyText; }
	const std::string& getBannerText() const { return m_BannerText; }
	bool isBubblyTextEnabled() const { return m_EnableBubblyText; }
	bool isGenTextBmpEnabled() const { return m_GenTextBmp; }

	const std::string& getBubblyTextConfig() const { return m_BubblyTextConfig; }
	void setBubblyTextConfig(const std::string& val) { m_BubblyTextConfig = val; }
	const std::string& getBubblyTextImage() const { return m_BubblyTextImage; }
	void setBubblyTextImage(const std::string& val) { m_BubblyTextImage = val; }
	const std::string& getBubblyTextShadowImage() const { return m_BubblyTextShadowImage; }
	void setBubblyTextShadowImage(const std::string& val) { m_BubblyTextShadowImage = val; }

	const std::string& getPhotoFolder() const { return m_PhotoFolder; }
	void setPhotoFolder(const std::string& val) { m_PhotoFolder = val; }

	const std::vector<ImageDownloader::ImageInfo>& getImageInfoList() const { return mImageInfoList; }

	int GetFPS() const { return m_FPS; }
	void SetFPS(int val) { m_FPS = val; }

private:
	void parse(const std::string& name, const std::string& value);

	std::string m_Version;
	std::string m_ProjectName;
	int m_ScreenWidth;
	int m_ScreenHeight;
	std::string m_LogPath;
	std::string m_BubblyText;
	std::string m_BannerText;
	std::string m_BubblyTextConfig;
	std::string m_BubblyTextImage;
	std::string m_BubblyTextShadowImage;
	bool m_EnableBubblyText;
	bool m_GenTextBmp;
	std::string m_PhotoFolder;
	std::vector<ImageDownloader::ImageInfo> mImageInfoList;
	bool m_LoadSuccess;
	int m_FPS;

};

