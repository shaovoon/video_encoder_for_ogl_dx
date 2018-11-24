#include "../include/GdiplusContext.h"

#ifdef _WIN32

GdiplusContext::GdiplusContext()
{
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);
}

GdiplusContext::~GdiplusContext()
{
	Gdiplus::GdiplusShutdown(m_gdiplusToken);
}

bool GdiplusContext::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	using namespace Gdiplus;

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return false;  // Failure

	pImageCodecInfo = (ImageCodecInfo*) (malloc(size));
	if (pImageCodecInfo == NULL)
		return false;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return true;  // Success
		}
	}

	free(pImageCodecInfo);
	return false;  // Failure
}

bool GdiplusContext::SavePngFile(const wchar_t* pszFile, Gdiplus::Bitmap* bmp) const
{
	if (bmp)
	{
		CLSID pngClsid;
		GetEncoderClsid(L"image/png", &pngClsid);
		Gdiplus::Status status = bmp->Save(pszFile, &pngClsid, NULL);
		return status == Gdiplus::Ok ? true : false;
	}

	return false;
}

bool GdiplusContext::SaveJpgFile(const wchar_t* pszFile, Gdiplus::Bitmap* bmp, ULONG quality) const
{
	if (bmp)
	{
		CLSID jpgClsid;
		GetEncoderClsid(L"image/jpeg", &jpgClsid);

		Gdiplus::EncoderParameters encoderParameters;
		encoderParameters.Count = 1;
		encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
		encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
		encoderParameters.Parameter[0].NumberOfValues = 1;
		encoderParameters.Parameter[0].Value = &quality;

		Gdiplus::Status status = bmp->Save(pszFile, &jpgClsid, &encoderParameters);
		return status == Gdiplus::Ok ? true : false;
	}

	return false;
}
#endif
