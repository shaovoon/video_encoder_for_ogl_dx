/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#include "Texture.h"
#include "constants.h"
#include "StringUtils.h"
#include "SceneException.h"
#include "Utility.h"

namespace Library
{

unsigned int Texture::sScreenWidth = 1000;
unsigned int Texture::sScreenHeight = 800;

Texture::Texture()
	: mTexture(0)
	, mWidth(0.0f)
	, mHeight(0.0f)
{
}

Texture::Texture(const std::string& folder, const std::string& image_file, float dim, bool gen_mipmap)
	: mTexture(0)
	, mWidth(dim)
	, mHeight(dim)
{
	LoadImageFile(folder, image_file, false, gen_mipmap);
	mUV.topUV = 0.0f;
	mUV.leftUV = 0.0f;
	mUV.rightUV = 1.0f;
	mUV.bottomUV = 1.0f;
}

Texture::Texture(const std::string& image_file, float dim, bool gen_mipmap)
	: mTexture(0)
	, mWidth(dim)
	, mHeight(dim)
{
	LoadImageFile(image_file, false, gen_mipmap);
	mUV.topUV = 0.0f;
	mUV.leftUV = 0.0f;
	mUV.rightUV = 1.0f;
	mUV.bottomUV = 1.0f;
}

Texture::~Texture()
{
	Destroy();
}

GLuint Texture::LoadSquareImageFile(const std::string& folder, const std::string& image_file, int dim, int actual_width, int actual_height, bool gen_mipmap)
{
	std::string path = folder;
	if (folder.size() > 0)
	{
		char ch = folder.at(folder.size() - 1);
		if (ch != '\\' && ch != '/')
			path += '/';
	}
	path += image_file;
	LoadImageFile(path, false, gen_mipmap);
	mUV.topUV = 0.0f;
	mUV.leftUV = 0.0f;
	mUV.rightUV = actual_width / (float)dim;
	mUV.bottomUV = actual_height / (float) dim;

	return mTexture;
}

GLuint Texture::LoadImageFile(const std::string& folder, const std::string& image_file, bool calculateUV, bool gen_mipmap)
{
	std::string path = folder;
	if (folder.size() > 0)
	{
		char ch = folder.at(folder.size()-1);
		if (ch != '\\' && ch != '/')
			path += '/';
	}
	path += image_file;

	return LoadImageFile(path, calculateUV, gen_mipmap);
}

#ifdef _MSC_VER
/*
GLuint Texture::LoadImageFile(const std::string& image_file, bool calculateUV, bool gen_mipmap)
{
	std::wstring image_file_wstr = toWString(image_file);
	using namespace Gdiplus;
	Bitmap canvas(image_file_wstr.c_str(), 1);
	return LoadImage(canvas, calculateUV, gen_mipmap);
}
*/
GLuint Texture::LoadImage(Gdiplus::Bitmap& canvas, bool calculateUV, bool gen_mipmap)
{
	using namespace Gdiplus;
	BitmapData bitmapData;
	uint32_t * pixels = LockBitmapData(canvas, bitmapData);
	if (!pixels)
		return 0;

	int stride = bitmapData.Stride >> 2;
	for (int row = 0; row < bitmapData.Height; ++row)
	{
		for (int col = 0; col < bitmapData.Width; ++col)
		{
			uint32_t index = (uint32_t)(row * stride + col);
			uint32_t a = (pixels[index] & 0xff000000) >> 24;
			uint32_t r = (pixels[index] & 0xff0000) >> 16;
			uint32_t g = (pixels[index] & 0xff00) >> 8;
			uint32_t b = (pixels[index] & 0xff);

			pixels[index] = a << 24 | b << 16 | g << 8 | r;
		}
	}

	GenTextureFromRawBytes(pixels, (GLsizei)canvas.GetWidth(), (GLsizei)canvas.GetHeight(), gen_mipmap, GL_RGBA);

	canvas.UnlockBits(&bitmapData);

	if (calculateUV)
		calculateDefUV();

	return mTexture;
}

uint32_t* Texture::LockBitmapData(Gdiplus::Bitmap& canvas, Gdiplus::BitmapData& bitmapData)
{
	using namespace Gdiplus;

	Rect rect(0, 0, (int)canvas.GetWidth(), (int)canvas.GetHeight());

	Status status = canvas.LockBits(
		&rect,
		ImageLockModeWrite,
		PixelFormat32bppARGB,
		&bitmapData);

	if (status != Ok)
		return nullptr;

	// Write to the temporary buffer provided by LockBits.
	uint32_t * pixels = (uint32_t*)bitmapData.Scan0;

	return pixels;
}

GLuint Texture::LoadImageFile(const std::string& image_file, bool calculateUV, bool gen_mipmap, uint32_t color_to_detect, int& min_height, int& max_height)
{
	std::wstring image_file_wstr = toWString(image_file);
	using namespace Gdiplus;
	Bitmap canvas(image_file_wstr.c_str(), 1);
	return LoadImage(canvas, calculateUV, gen_mipmap, color_to_detect, min_height, max_height);
}

GLuint Texture::LoadImage(Gdiplus::Bitmap& canvas, bool calculateUV, bool gen_mipmap, uint32_t color_to_detect, int& min_height, int& max_height)
{
	using namespace Gdiplus;
	BitmapData bitmapData;
	uint32_t * pixels = LockBitmapData(canvas, bitmapData);
	if (!pixels)
		return 0;

	int stride = bitmapData.Stride >> 2;
	uint32_t dr = (color_to_detect & 0xff0000) >> 16;
	uint32_t dg = (color_to_detect & 0xff00) >> 8;
	uint32_t db = (color_to_detect & 0xff);

	min_height = 1000000;
	max_height = 0;

	for (int row = 0; row < bitmapData.Height; ++row)
	{
		for (int col = 0; col < bitmapData.Width; ++col)
		{
			uint32_t index = (uint32_t)(row * stride + col);
			uint32_t a = (pixels[index] & 0xff000000) >> 24;
			uint32_t r = (pixels[index] & 0xff0000) >> 16;
			uint32_t g = (pixels[index] & 0xff00) >> 8;
			uint32_t b = (pixels[index] & 0xff);
			if (dr == r && dg == g && db == b)
			{
				if (row < min_height)
				{
					min_height = row;
				}
				if (row > max_height)
				{
					max_height = row;
				}
			}

			pixels[index] = a << 24 | b << 16 | g << 8 | r;
		}
	}

	GenTextureFromRawBytes(pixels, (GLsizei)canvas.GetWidth(), (GLsizei)canvas.GetHeight(), gen_mipmap, GL_RGBA);

	canvas.UnlockBits(&bitmapData);

	if (calculateUV)
		calculateDefUV();

	return mTexture;
}
#endif
GLuint Texture::LoadImageFile(const std::string& image_file, bool calculateUV, bool gen_mipmap)
{
	//Load the image from the file into SDL's surface representation
	SDL_Surface* surface = IMG_Load(image_file.c_str());
	if (surface == NULL) { //If it failed, say why and don't continue loading the texture
		printf("Error: \"%s\"\n", SDL_GetError()); return 0;
	}

	int mode = GL_RGB;
	if (surface->format->BytesPerPixel == 4)
	{
		mode = GL_RGBA;
	}

	GenTextureFromRawBytes((uint32_t *) surface->pixels, surface->w, surface->h, gen_mipmap, mode);

	//Unload SDL's copy of the data; we don't need it anymore because OpenGL now stores it in the texture.
	SDL_FreeSurface(surface);

	if (calculateUV)
		calculateDefUV();

	return mTexture;
}
GLuint Texture::LoadImageFromMemory(char* buf, size_t size, bool calculateUV, bool gen_mipmap)
{
	SDL_RWops * src = SDL_RWFromMem((void*)buf, (int) size);

	//Load the image from the file into SDL's surface representation
	SDL_Surface* surface = IMG_Load_RW(src, 1);
	if (surface == NULL) { //If it failed, say why and don't continue loading the texture
		printf("Error: \"%s\"\n", SDL_GetError()); return 0;
	}

	int mode = GL_RGB;
	if (surface->format->BytesPerPixel == 4)
	{
		mode = GL_RGBA;
	}

	GenTextureFromRawBytes((uint32_t *)surface->pixels, surface->w, surface->h, gen_mipmap, mode);

	//Unload SDL's copy of the data; we don't need it anymore because OpenGL now stores it in the texture.
	SDL_FreeSurface(surface);

	if (calculateUV)
		calculateDefUV();

	return mTexture;
}

void Texture::GenTextureFromRawBytes(uint32_t * pixels, GLsizei width, GLsizei height, bool gen_mipmap, int mode)
{
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, mode, width, height, 0, mode, GL_UNSIGNED_BYTE, pixels);
	//Set the minification and magnification filters.  In this case, when the texture is minified (i.e., the texture's pixels (texels) are
	//*smaller* than the screen pixels you're seeing them on, linearly filter them (i.e. blend them together).  This blends four texels for
	//each sample--which is not very much.  Mipmapping can give better results.  Find a texturing tutorial that discusses these issues
	//further.  Conversely, when the texture is magnified (i.e., the texture's texels are *larger* than the screen pixels you're seeing
	//them on), linearly filter them.  Qualitatively, this causes "blown up" (overmagnified) textures to look blurry instead of blocky.
	if (gen_mipmap)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	if (!gen_mipmap)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
}

void Texture::Destroy()
{
	if (mTexture != 0)
	{
		glDeleteTextures(1, &mTexture);
	}
	mTexture = 0;
}

void Texture::setUV(uv val)
{
	mUV = val;
}

Texture::uv Texture::getUV()
{
	return mUV;
}

void Texture::calculateDefUV()
{
	mUV.topUV = 0.0f;
	mUV.leftUV = 0.0f;

	float screen_h_ratio = sScreenHeight / (float) sScreenWidth;
	float image_h_ratio = mHeight / mWidth;

	float screen_w_ratio = sScreenWidth / (float) sScreenHeight;
	float image_w_ratio = mWidth / mHeight;

	if (mWidth >= mHeight)
	{
		if (screen_h_ratio >= image_h_ratio)
		{
			mUV.rightUV = 1.0f - (screen_h_ratio - image_h_ratio);
			mUV.bottomUV = image_h_ratio;
		}
		else
		{
			mUV.rightUV = 1.0f;
			mUV.bottomUV = screen_h_ratio;
		}
	}
	else if (mWidth < mHeight)
	{
		if (screen_w_ratio >= image_w_ratio)
		{
			mUV.rightUV = image_w_ratio;
			mUV.bottomUV = 1.0f - (screen_w_ratio - image_w_ratio);
		}
		else
		{
			mUV.rightUV = screen_w_ratio;
			mUV.bottomUV = 1.0f;
		}
	}
}

void Texture::LoadImageFile(const std::string& image_file, Texture& texture, char xorbyte)
{
	const std::string ext = Library::Utility::GetFileExtension(image_file);
	if (ext == ".bin" || ext == ".txt") // cannot send bin over web, this is workaround
	{
		char* buf = nullptr;
		size_t size = 0;
		if (ImageEncoder::LoadingEncodedFileIntoPtr(image_file, xorbyte, buf, size))
		{
			if (texture.LoadImageFromMemory(buf, size, true, false) == 0)
			{
				delete[] buf;
				std::string err = "texture load failed: ";
				err += image_file;
				throw Library::SceneException(err.c_str());
			}
		}
		delete[] buf;
	}
	else if (texture.LoadImageFile(image_file, true, false) == 0)
	{
		std::string err = "texture load failed: ";
		err += image_file;
		throw Library::SceneException(err.c_str());
	}
	return;
}

} // ns Library