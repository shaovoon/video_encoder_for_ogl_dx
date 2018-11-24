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

	const std::string& GetVersion() const { return m_Version; }
	const std::string& GetProjectName() const { return m_ProjectName; }
	int GetScreenWidth() const { return m_ScreenWidth; }
	int GetScreenHeight() const { return m_ScreenHeight; }
	const std::string& GetLogPath() const { return m_LogPath; }

	int GetFPS() const { return m_FPS; }
	void SetFPS(int val) { m_FPS = val; }

private:
	void Parse(const std::string& name, const std::string& value);

	std::string m_Version;
	std::string m_ProjectName;
	int m_ScreenWidth;
	int m_ScreenHeight;
	std::string m_LogPath;
	bool m_LoadSuccess;
	int m_FPS;
};

