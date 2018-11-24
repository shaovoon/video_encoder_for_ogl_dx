#pragma once

#include <string>
namespace Library
{
	class ImageEncoder
	{
	public:
		ImageEncoder();
		~ImageEncoder();

		static char* ReadWholeFile(const std::string& file, size_t& size);
		static bool WriteWholeFile(const std::string& file, char* buf, size_t& size);

		static bool BitwiseXOR(char* buf, size_t& size, char byte);

		static bool LoadingEncodedFileIntoPtr(const std::string& file, char xor_byte, char*& buf, size_t& size);
		static bool EncodeFile(const std::string& src_file, const std::string& dest_file, char xor_byte);
	};

} // ns Library