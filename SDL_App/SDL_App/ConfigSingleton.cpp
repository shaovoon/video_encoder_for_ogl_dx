#include "ConfigSingleton.h"
#include "csv_stream.h"
#include "StringUtils.h"
#include "DownloadSingleton.h"

extern Library::DownloadSingleton gDownloadSingleton;

RTTI_DEFINITIONS(ConfigSingleton)

ConfigSingleton::ConfigSingleton()
	: m_ScreenWidth(800)
	, m_ScreenHeight(600)
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
			Parse(name, value);
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
		os << "ScreenWidth" << m_ScreenWidth << NEWLINE;
		os << "ScreenHeight" << m_ScreenHeight << NEWLINE;
		os << "LogPath" << m_LogPath << NEWLINE;
		os << "FPS" << m_FPS << NEWLINE;
		//os << << << NEWLINE;

		os.flush();
		os.close();
		return true;
	}

	return false;
}


void ConfigSingleton::Parse(const std::string& name, const std::string& value)
{
	if (name == "ScreenWidth")
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
	else if (name == "FPS")
	{
		m_FPS = atoi(value.c_str());
	}
}

void ConfigSingleton::DownloadFiles()
{
	printf("SRC_FOLDER:%s\n", gDownloadSingleton.getSrcFolder().c_str());
	gDownloadSingleton.DownloadFile(this, gDownloadSingleton.getSrcFolder(), "config.txt",
		Library::DownloadableComponent::FileType::INI_FILE, false);
}

bool ConfigSingleton::OpenFile(const char* file, FileType file_type)
{
	if (file_type == Library::DownloadableComponent::FileType::INI_FILE)
	{
		LoadConfigFile(file);
		SetInitialized(true);
		return true;
	}

	return false;
}
