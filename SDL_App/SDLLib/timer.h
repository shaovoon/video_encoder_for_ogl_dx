#pragma once
#include <chrono>
#include <cstdlib>
#include <cstdio>

class timer
{
public:
	timer() = default;
	void start(const std::string& text_)
	{
		text = text_;
		begin = std::chrono::system_clock::now();
	}
	void stop()
	{
		auto end = std::chrono::system_clock::now();
		auto dur = end - begin;
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
		using namespace std;
		printf("%s:%d ms", text.c_str(), ms);
	}

private:
	std::string text;
	std::chrono::system_clock::time_point begin;
};
