#pragma once
#include <string>
#include <cstdio>
#include <vector>

struct FileJoiner
{
private:
	static char* read_whole_file(const std::string& src_file, int& total_size)
	{
#ifdef _MSC_VER
		FILE *f = NULL;
		fopen_s(&f, src_file.c_str(), "rb");
#else
		FILE* f = fopen(src_file.c_str(), "rb");
#endif
		if(!f)
			return NULL;

		fseek(f, 0, SEEK_END);
		total_size = ftell(f);
		fseek(f, 0, SEEK_SET);  //same as rewind(f);

		char *arr = new char[total_size];
		fread(arr, total_size, 1, f);
		fclose(f);

		return arr;
	}

	static bool write_whole_file(const std::string& dest_file, const char* arr, const int total_size)
	{
#ifdef _MSC_VER
		FILE *f = NULL;
		fopen_s(&f, dest_file.c_str(), "wb");
#else
		FILE *f = fopen(dest_file.c_str(), "wb");
#endif
		if(!f)
			return false;

		fwrite(arr, total_size, 1, f);
		fflush(f);
		fclose(f);

		return true;
	}
public:
	static bool gather(const std::vector<std::string>& vecFiles, std::vector<size_t>& vecSize, const std::string& destFile)
	{
		vecSize.clear();
#ifdef _MSC_VER
		FILE *f = NULL;
		fopen_s(&f, destFile.c_str(), "wb");
#else
		FILE *f = fopen(destFile.c_str(), "wb");
#endif
		if(!f)
			return false;

		for(size_t i=0; i<vecFiles.size(); ++i)
		{
			int total_size = 0;
			char* arr = read_whole_file(vecFiles[i], total_size);

			if(arr==NULL)
				return false;

			if(total_size==0)
				return false;

			fwrite(arr, total_size, 1, f);

			vecSize.push_back(total_size);

			delete [] arr;
		}

		fflush(f);
		fclose(f);
		return true;
	}

	static bool scatter(const std::vector<std::string>& vecFiles, const std::vector<size_t>& vecSize, const std::string& srcFile)
	{
		int total_size=0;
		char* arr = read_whole_file(srcFile, total_size);

		if(arr==NULL)
			return false;

		char* start=arr;
		for(size_t i=0; i<vecFiles.size(); ++i)
		{
			write_whole_file(vecFiles[i], start, vecSize[i]);
			start += vecSize[i];
		}

		delete [] arr;

		return true;
	}

	static bool write_file_size(const std::string& dest_file, const std::vector<size_t>& vecSize)
	{
#ifdef _MSC_VER
		FILE *f = NULL;
		fopen_s(&f, dest_file.c_str(), "w");
#else
		FILE *f = fopen(dest_file.c_str(), "w");
#endif
		if(!f)
			return false;

		fprintf(f, "std::vector<size_t> vecSize = {");
		for(size_t i=0; i< vecSize.size(); ++i)
		{
			if(i== vecSize.size()-1)
				fprintf(f, "%lu ", vecSize[i]);
			else
				fprintf(f, "%lu, ", vecSize[i]);
		}
		fprintf(f, "};\n");

		fflush(f);
		fclose(f);

		return true;
	}


};