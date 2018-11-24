#include "ConfigSingleton.h"
#include "csv_stream.h"
#include "StringUtils.h"
#include "DownloadSingleton.h"

extern Library::DownloadSingleton gDownloadSingleton;

RTTI_DEFINITIONS(ConfigSingleton)

ConfigSingleton::ConfigSingleton()
	: m_ScreenWidth(800)
	, m_ScreenHeight(600)
	, m_EnableBubblyText(false)
	, m_GenTextBmp(false)
	, m_LoadSuccess(false)
	, m_FPS(30)
{
}

ConfigSingleton::~ConfigSingleton()
{
}

bool ConfigSingleton::LoadConfigFile(const std::string& file)
{
	m_LoadSuccess = false;

	capi::csv::ifstream is(file.c_str());
	is.set_delimiter('=', "#equal;");
	if (is.is_open())
	{
		while (is.read_line())
		{
			std::string name, value;
			is >> name;
			value = is.get_rest_of_line();

			trim(name);
			trim(value);
			parse(name, value);
		}
		m_LoadSuccess = true; // TODO: port validation checks from other app
		return true;
	}
	return false;
}

bool ConfigSingleton::WriteConfigFile(const std::string& file)
{
	capi::csv::ofstream os(file.c_str());
	os.set_delimiter('=', "#equal;");
	if (os.is_open())
	{
		os << "Version" << m_Version << NEWLINE;
		os << "ProjectName" << m_ProjectName << NEWLINE;
		os << "ScreenWidth" << m_ScreenWidth << NEWLINE;
		os << "ScreenHeight" << m_ScreenHeight << NEWLINE;
		os << "LogPath" << m_LogPath << NEWLINE;
		os << "BubblyText" << m_BubblyText << NEWLINE;
		os << "BannerText" << m_BannerText << NEWLINE;
		os << "EnableBubblyText" << (m_EnableBubblyText ? "true" : "false") << NEWLINE;
		os << "BubblyTextConfig" << m_BubblyTextConfig << NEWLINE;
		os << "BubblyTextImage" << m_BubblyTextImage << NEWLINE;
		os << "BubblyTextShadowImage" << m_BubblyTextShadowImage << NEWLINE;
		os << "GenTextBmp" << (m_GenTextBmp ? "true" : "false") << NEWLINE;
		os << "PhotoFolder" << m_PhotoFolder << NEWLINE;
		os << "FPS" << m_FPS << NEWLINE;
		//os << << << NEWLINE;

		os.flush();
		os.close();
		return true;
	}

	return false;
}


void ConfigSingleton::parse(const std::string& name, const std::string& value)
{
	if (name == "ImageInfo")
	{
		capi::csv::istringstream ifs(value.c_str());
		ifs.set_delimiter(',', "#comma;");
		int r = 0, g = 0, b = 0;
		if (ifs.read_line())
		{
			ImageDownloader::ImageInfo info;
			ifs >> info.mFile >> info.mDim >> info.mWidth >> info.mHeight;
			mImageInfoList.push_back(info);
		}
	}
	else if (name == "Version")
	{
		m_Version = value;
	}
	else if (name == "ProjectName")
	{
		m_ProjectName = value;
	}
	else if (name == "ScreenWidth")
	{
		m_ScreenWidth = atoi(value.c_str());
	}
	else if (name == "ScreenHeight")
	{
		m_ScreenHeight = atoi(value.c_str());
	}
	else if (name == "LogPath")
	{
		m_LogPath = value;
	}
	else if (name == "BubblyText")
	{
		m_BubblyText = value;
	}
	else if (name == "BannerText")
	{
		m_BannerText = value;
	}
	else if (name == "EnableBubblyText")
	{
		m_EnableBubblyText = ((value == "true") || (value == "1"));
	}
	else if (name == "BubblyTextConfig")
	{
		m_BubblyTextConfig = value;
	}
	else if (name == "BubblyTextImage")
	{
		m_BubblyTextImage = value;
	}
	else if (name == "BubblyTextShadowImage")
	{
		m_BubblyTextShadowImage = value;
	}
	else if (name == "GenTextBmp")
	{
		m_GenTextBmp = ((value == "true") || (value == "1"));
	}
	else if (name == "PhotoFolder")
	{
		m_PhotoFolder = value;
	}
	else if (name == "FPS")
	{
		m_FPS = atoi(value.c_str());
	}
}

void ConfigSingleton::DownloadFiles()
{
	gDownloadSingleton.DownloadFile(this, gDownloadSingleton.getSrcFolder(), "config.txt",
		Library::DownloadableComponent::FileType::INI_FILE, false);
}

bool ConfigSingleton::OpenFile(const char* file, FileType file_type)
{
	if (file_type == Library::DownloadableComponent::FileType::INI_FILE)
	{
		if (LoadConfigFile(file))
		{
			SetInitialized(true);
		}
		return true;
	}

	return false;
}
