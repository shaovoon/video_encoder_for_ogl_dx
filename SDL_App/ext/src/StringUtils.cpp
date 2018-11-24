//#include "stdafx.h"
#include "../include/StringUtils.h"
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include "../include/UTF8Utils/utf8.h"

std::wstring toWString(const std::string& text)
{
	std::wstring wtext=L"";
	wtext.reserve(text.size() + 1);
	for (size_t i = 0; i < text.size(); ++i)
	{
		wtext += (wchar_t)text[i];
	}
	return wtext;
}

std::string toAString(const std::wstring& wtext)
{
	std::string text = "";
	text.reserve(text.size() + 1);
	for (size_t i = 0; i < wtext.size(); ++i)
	{
		text += (char) wtext[i];
	}
	return text;
}

// trim from start
std::string& leftTrim(std::string &s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
std::string& rightTrim(std::string &s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
std::string& trim(std::string &s) 
{
	return leftTrim(rightTrim(s));
}

std::string optimized_trim(const std::string& str, const std::string& trimChars)
{
	std::string result = "";

	if (str.empty())
		return result;

	size_t startpos = str.find_first_not_of(trimChars);
	size_t endpos = str.find_last_not_of(trimChars);

	if (std::string::npos == startpos&&std::string::npos == endpos)
		return str;

	startpos = (std::string::npos == startpos) ? 0 : startpos;
	endpos = (std::string::npos == endpos) ? str.size() : endpos;

	if (startpos <= endpos)
	{
		result = str.substr(startpos, endpos - startpos + 1);
	}

	return result;
}

std::wstring optimized_trim(const std::wstring& str, const std::wstring& trimChars)
{
	std::wstring result = L"";

	if (str.empty())
		return result;

	size_t startpos = str.find_first_not_of(trimChars);
	size_t endpos = str.find_last_not_of(trimChars);

	if (std::wstring::npos == startpos && std::wstring::npos == endpos)
		return str;

	startpos = (std::wstring::npos == startpos) ? 0 : startpos;
	endpos = (std::wstring::npos == endpos) ? str.size() : endpos;

	if (startpos <= endpos)
	{
		result = str.substr(startpos, endpos - startpos + 1);
	}

	return result;
}

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr)
{
	std::string utf8line;

	if (wstr.empty())
		return utf8line;

#ifdef _MSC_VER
	utf8::utf16to8(wstr.begin(), wstr.end(), std::back_inserter(utf8line));
#else
	utf8::utf32to8(wstr.begin(), wstr.end(), std::back_inserter(utf8line));
#endif
	return utf8line;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
	std::wstring wide_line;

	if (str.empty())
		return wide_line;

#ifdef _MSC_VER
	utf8::utf8to16(str.begin(), str.end(), std::back_inserter(wide_line));
#else
	utf8::utf8to32(str.begin(), str.end(), std::back_inserter(wide_line));
#endif
	return wide_line;
}

std::vector<std::string> split_string(const std::string& str, const std::string& delimiters, bool trimEmpty)
{
	std::vector<std::string> tokens;
	std::string::size_type pos, lastPos = 0, length = str.length();

	using value_type = typename std::vector<std::string>::value_type;
	using size_type = typename std::vector<std::string>::size_type;

	while (lastPos < length + 1)
	{
		pos = str.find_first_of(delimiters, lastPos);
		if (pos == std::string::npos)
		{
			pos = length;
		}

		if (pos != lastPos || !trimEmpty)
			tokens.push_back(value_type(str.data() + lastPos,
			(size_type)pos - lastPos));

		lastPos = pos + 1;
	}
	return tokens;
}

std::vector<std::wstring> split_string(const std::wstring& str, const std::wstring& delimiters, bool trimEmpty)
{
	std::vector<std::wstring> tokens;
	std::wstring::size_type pos, lastPos = 0, length = str.length();

	using value_type = typename std::vector<std::wstring>::value_type;
	using size_type = typename std::vector<std::wstring>::size_type;

	while (lastPos < length + 1)
	{
		pos = str.find_first_of(delimiters, lastPos);
		if (pos == std::wstring::npos)
		{
			pos = length;
		}

		if (pos != lastPos || !trimEmpty)
			tokens.push_back(value_type(str.data() + lastPos,
			(size_type)pos - lastPos));

		lastPos = pos + 1;
	}
	return tokens;
}

bool is_inside(const std::string& str, const std::string& to_find)
{
	return (str.find_first_of(to_find, 0) != std::string::npos);
}

bool is_inside(const std::wstring& str, const std::wstring& to_find)
{
	return (str.find_first_of(to_find, 0) != std::wstring::npos);
}
