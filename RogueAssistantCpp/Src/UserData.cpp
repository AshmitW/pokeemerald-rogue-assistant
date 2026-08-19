#include "UserData.h"
#include "StringUtils.h"
#include "Log.h"

#include <cstdlib>
#include <filesystem>
#include <system_error>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4244)
#endif

namespace fs = std::filesystem;

std::map<std::string, std::string> UserData::s_SavedValues;
bool UserData::s_PendingSavedValueChange = false;

// Windows kept user data under %appdata%/.pokabbie/rogue_assistant.
// Linux follows the XDG base directory spec instead.
static fs::path GetUserDataRoot()
{
#ifdef _WIN32
	wchar_t* buffer = nullptr;
	size_t size = 0;
	if (_wdupenv_s(&buffer, &size, L"appdata") == 0 && buffer != nullptr)
	{
		fs::path root = fs::path(buffer) / L".pokabbie" / L"rogue_assistant";
		free(buffer);
		return root;
	}
	return fs::path(L"save") / L".pokabbie" / L"rogue_assistant";
#else
	char const* xdgDataHome = std::getenv("XDG_DATA_HOME");
	if (xdgDataHome != nullptr && *xdgDataHome != '\0')
		return fs::path(xdgDataHome) / "pokabbie" / "rogue_assistant";

	char const* home = std::getenv("HOME");
	if (home != nullptr && *home != '\0')
		return fs::path(home) / ".local" / "share" / "pokabbie" / "rogue_assistant";

	return fs::path("save") / "pokabbie" / "rogue_assistant";
#endif
}

// Callers pass relative paths such as L"2/1234567/boxes.dat"; absolute paths
// are used as-is. std::filesystem handles separators, so the manual backslash
// rewriting the Windows version did is no longer needed.
static fs::path FormatPath(std::wstring const& path)
{
	fs::path input(path);

	if (input.is_absolute())
		return input.lexically_normal();

	return (GetUserDataRoot() / input).lexically_normal();
}

bool UserData::DoesDirectoryExist(std::wstring const& path)
{
	std::error_code errorCode;
	return fs::is_directory(FormatPath(path), errorCode);
}

bool UserData::DoesFileExist(std::wstring const& path)
{
	std::error_code errorCode;
	return fs::is_regular_file(FormatPath(path), errorCode);
}

static void EnsureParentDirectoriesExist(fs::path const& fullPath)
{
	fs::path parent = fullPath.parent_path();
	if (parent.empty())
		return;

	std::error_code errorCode;
	if (!fs::is_directory(parent, errorCode))
	{
		LOG_INFO("UserData::CreateDir %s", parent.string().c_str());
		fs::create_directories(parent, errorCode);
	}
}

bool UserData::TryOpenReadFile(std::wstring const& inPath, std::fstream& outStream)
{
	if (DoesFileExist(inPath))
	{
		fs::path fullPath = FormatPath(inPath);
		EnsureParentDirectoriesExist(fullPath);

		LOG_INFO("UserData::OpenRead %s", fullPath.string().c_str());

		outStream.close();
		outStream.open(fullPath, std::ios::binary | std::ios::in);
		return outStream.is_open();
	}

	return false;
}

bool UserData::TryOpenWriteFile(std::wstring const& inPath, std::fstream& outStream, bool createIfMissing)
{
	if (createIfMissing || DoesFileExist(inPath))
	{
		fs::path fullPath = FormatPath(inPath);
		EnsureParentDirectoriesExist(fullPath);

		LOG_INFO("UserData::OpenWrite %s", fullPath.string().c_str());

		outStream.close();
		outStream.open(fullPath, std::ios::binary | std::ios::out | std::ios::trunc);
		return outStream.is_open();
	}

	return false;
}

bool UserData::TryOpenAppendFile(std::wstring const& inPath, std::fstream& outStream, bool createIfMissing)
{
	if (createIfMissing || DoesFileExist(inPath))
	{
		fs::path fullPath = FormatPath(inPath);
		EnsureParentDirectoriesExist(fullPath);

		LOG_INFO("UserData::OpenAppend %s", fullPath.string().c_str());

		outStream.close();
		outStream.open(fullPath, std::ios::out | std::ios::app);
		return outStream.is_open();
	}

	return false;
}

void UserData::Init()
{
	std::ifstream stream("settings.ini");

	if (stream.is_open())
	{
		std::string line;
		while (std::getline(stream, line))
		{
			std::vector<std::string> parts = strutil::split(line, '=');
			if (parts.size() == 2)
			{
				SetSavedString(parts[0], parts[1]);
			}
		}
		stream.close();
	}
}

void UserData::Update()
{
	if (s_PendingSavedValueChange)
	{
		s_PendingSavedValueChange = false;
		std::ofstream stream("settings.ini");

		if (stream.is_open())
		{
			for (auto it : s_SavedValues)
			{
				stream << it.first << "=" << it.second << "\n";
			}

			stream.close();
		}
	}
}

std::string UserData::GetSavedString(std::string const& key, std::string const& defaultValue)
{
	auto it = s_SavedValues.find(key);
	if (it == s_SavedValues.end())
	{
		SetSavedString(key, defaultValue);
		return defaultValue;
	}

	return it->second;
}

void UserData::SetSavedString(std::string const& key, std::string const& value)
{
	s_SavedValues[key] = value;
	s_PendingSavedValueChange = true;
}

int UserData::GetSavedInt(std::string const& key, int defaultValue)
{
	std::string strValue = GetSavedString(key, std::to_string(defaultValue));

	try 
	{
		return std::stoi(strValue);
	}
	catch (std::exception) //& e)
	{
		SetSavedInt(key, defaultValue);
		return defaultValue;
	}
}

void UserData::SetSavedInt(std::string const& key, int value)
{
	SetSavedString(key, std::to_string(value));
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif