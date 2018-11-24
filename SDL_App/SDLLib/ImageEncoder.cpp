
#include "ImageEncoder.h"
#include "Texture.h"

namespace Library
{

ImageEncoder::ImageEncoder()
{
}

ImageEncoder::~ImageEncoder()
{
}

bool ImageEncoder::EncodeFile(const std::string& src_file, const std::string& dest_file, char xor_byte)
{
	size_t size = 0;
	char* buf = ReadWholeFile(src_file, size);
	if (!buf)
		return false;
	if (!size)
		return false;
	BitwiseXOR(buf, size, 0b00100000);
	bool ret = WriteWholeFile(dest_file, buf, size);

	delete[] buf;

	return ret;
}

bool ImageEncoder::LoadingEncodedFileIntoPtr(const std::string& file, char xor_byte, char*& buf, size_t& size)
{
	size = 0;
	buf = ReadWholeFile(file, size);
	if (!buf)
		return false;
	if (!size)
		return false;
	BitwiseXOR(buf, size, xor_byte);

	return true;
}

char* ImageEncoder::ReadWholeFile(const std::string& file, size_t& size)
{
	size = 0;
#ifdef _MSC_VER
	FILE *f = nullptr;
	fopen_s(&f, file.c_str(), "rb");
#else
	FILE *f = fopen(file.c_str(), "rb");
#endif

	if (!f)
		return nullptr;

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buf = nullptr;
	try
	{
		buf = new char[size];
	}
	catch (std::bad_alloc&)
	{
		fclose(f);

		return nullptr;
	}
	if (!buf)
	{
		fclose(f);

		return nullptr;
	}
	fread(buf, size, 1, f);
	fclose(f);

	return buf;
}

bool ImageEncoder::WriteWholeFile(const std::string& file, char* buf, size_t& size)
{
	if (!buf)
		return false;

#ifdef _MSC_VER
	FILE *f = nullptr;
	fopen_s(&f, file.c_str(), "wb");
#else
	FILE *f = fopen(file.c_str(), "wb");
#endif

	if (!f)
		return false;

	fwrite(buf, size, 1, f);
	fflush(f);
	fclose(f);

	return true;
}

bool ImageEncoder::BitwiseXOR(char* buf, size_t& size, char byte)
{
	if (!buf)
		return false;
	if (size == 0)
		return false;
	for (size_t i = 0; i < size; ++i)
	{
		buf[i] ^= byte;
	}
	return true;
}

} // ns Library