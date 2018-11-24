#pragma once
#ifdef _WIN32
//#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

class GdiplusContext
{
public:
	GdiplusContext();
	~GdiplusContext();

	bool SavePngFile(const wchar_t* pszFile, Gdiplus::Bitmap* bmp) const;
	bool SaveJpgFile(const wchar_t* pszFile, Gdiplus::Bitmap* bmp, ULONG quality) const; // quality = [0..100]

private:
	static bool GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

	Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
	ULONG_PTR m_gdiplusToken;

};
#endif
