#pragma once

#include <string>
#include <vector>

std::wstring toWString(const std::string& text);
std::string toAString(const std::wstring& wtext);

std::string& leftTrim(std::string &s);

// trim from end
std::string& rightTrim(std::string &s);

// trim from both ends
std::string& trim(std::string &s);

std::string optimized_trim(const std::string& str, const std::string& trimChars="\r\n\t ");
std::wstring optimized_trim(const std::wstring& str, const std::wstring& trimChars = L"\r\n\t ");

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr);

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str);

std::vector<std::string> split_string(const std::string& str, const std::string& delimiters = ",", bool trimEmpty = false);
std::vector<std::wstring> split_string(const std::wstring& str, const std::wstring& delimiters = L",", bool trimEmpty = false);

bool is_inside(const std::string& str, const std::string& to_find);
bool is_inside(const std::wstring& str, const std::wstring& to_find);
