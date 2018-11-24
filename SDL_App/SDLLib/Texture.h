/********************************/
/* SDLLib Emscripten Framework  */
/* Copyright © 2016             */
/* Author: Wong Shao Voon       */
/********************************/
#pragma once

#include <string>
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_video.h>
#include <SDL_image.h> //Needed for IMG_Load.  If you want to use bitmaps (SDL_LoadBMP), it appears to not be necessary

#ifdef _MSC_VER
#include <windows.h>
#define NOMINMAX
#include <algorithm>
namespace Gdiplus
{
	using std::min;
	using std::max;
}
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#endif

#include "ImageEncoder.h"
namespace Library
{

class Texture
{
public:
	Texture();
	Texture(const std::string& folder, const std::string& image_file, float width, bool gen_mipmap);
	Texture(const std::string& image_file, float width, bool gen_mipmap);
	~Texture();

	GLuint LoadSquareImageFile(const std::string& folder, const std::string& image_file, int dim, int actual_width, int actual_height, bool gen_mipmap);
	GLuint LoadImageFile(const std::string& folder, const std::string& image_file, bool calculateUV, bool gen_mipmap);
	GLuint LoadImageFile(const std::string& image_file, bool calculateUV, bool gen_mipmap);

#ifdef _MSC_VER
	GLuint LoadImage(Gdiplus::Bitmap& canvas, bool calculateUV, bool gen_mipmap);
	GLuint LoadImageFile(const std::string& image_file, bool calculateUV, bool gen_mipmap, uint32_t color_to_detect, int& min_height, int& max_height);
	GLuint LoadImage(Gdiplus::Bitmap& canvas, bool calculateUV, bool gen_mipmap, uint32_t color_to_detect, int& min_height, int& max_height);
	uint32_t* LockBitmapData(Gdiplus::Bitmap& canvas, Gdiplus::BitmapData& bitmapData);
#endif

	static void LoadImageFile(const std::string& image_file, Texture& texture, char xorbyte);
	GLuint LoadImageFromMemory(char* buf, size_t size, bool calculateUV, bool gen_mipmap);

	void GenTextureFromRawBytes(uint32_t * pixels, GLsizei width, GLsizei height, bool gen_mipmap, int mode);

	void Destroy();
	GLuint getTexture() const { return mTexture; }
	float getWidth() const { return mWidth; }
	void setWidth(float val) { mWidth = val; }
	float getHeight() const { return mHeight; }
	void setHeight(float val) { mHeight = val; }

	struct uv
	{
		uv() : leftUV(0.0f), topUV(0.0f), rightUV(1.0f), bottomUV(1.0f) {}
		uv(float _leftUV, float _topUV, float _rightUV, float _bottomUV) 
			: leftUV(_leftUV), topUV(_topUV), rightUV(_rightUV), bottomUV(_bottomUV) {}
		float leftUV;
		float topUV;
		float rightUV;
		float bottomUV;
	};

	void setUV(uv val);
	uv getUV();

	void calculateDefUV();

	static void setScreenDim(unsigned int screenWidth, unsigned int screenHeight)
	{
		sScreenWidth = screenWidth;
		sScreenHeight = screenHeight;
	}

private:
	GLuint mTexture;
	float mWidth; // width of the original image
	float mHeight; // height of the original image
	uv mUV;
	static unsigned int sScreenWidth;
	static unsigned int sScreenHeight;
};

} // ns Library