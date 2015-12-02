#include "stdafx.h"
#include "LoadImage_vcl.h"

namespace vgext
{
	ImageLoader ImageLoader::instance;

	ImageLoader::ImageLoader() : gdiplusToken(NULL), gdiplusStartupInput()
	{
		Gdiplus::GdiplusStartup(&(this->gdiplusToken), &(this->gdiplusStartupInput), NULL);
	}

	ImageLoader::~ImageLoader()
	{
	}

	ImageLoader & ImageLoader::getInstance()
	{
		return instance;
	}

	VGImage ImageLoader::loadImageFromFile(LPCTSTR fn)
	{
		Gdiplus::Bitmap bitmap(fn);
		if (bitmap.GetLastStatus() != Gdiplus::Ok) {
			return NULL;
		}
		
		const UINT width = bitmap.GetWidth();
		const UINT height = bitmap.GetHeight();

		Gdiplus::BitmapData bitmapData;
		bitmap.LockBits(NULL, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bitmapData);
		LPBYTE cimg = reinterpret_cast<LPBYTE>(bitmapData.Scan0);

		if (bitmapData.Stride <= 0) {
			bitmap.UnlockBits(&bitmapData);
			return NULL;
		}

		UINT pitch = static_cast<UINT>(bitmapData.Stride);
		LPBYTE nimg = reinterpret_cast<LPBYTE>(::LocalAlloc(LPTR, pitch * height));
		for (UINT y = 0u; y < height; y++) {
			for (UINT x = 0u; x < width; x++) {
				nimg[y*pitch+x*4u+0u] = cimg[(height-y-1u)*pitch+x*4u+3u];	// A
				nimg[y*pitch+x*4u+1u] = cimg[(height-y-1u)*pitch+x*4u+0u];	// B
				nimg[y*pitch+x*4u+2u] = cimg[(height-y-1u)*pitch+x*4u+1u];	// G
				nimg[y*pitch+x*4u+3u] = cimg[(height-y-1u)*pitch+x*4u+2u];	// R
			}
		}

		VGImage vgimage = ::vgCreateImage(VG_sRGBA_8888, width, height, VG_IMAGE_QUALITY_BETTER);
		::vgImageSubData(vgimage, nimg, pitch, VG_sRGBA_8888, 0, 0, width, height);
		::LocalFree(nimg);
		bitmap.UnlockBits(&bitmapData);

		return vgimage;
	}
}

void vgextPrintError(LPCTSTR filepath, int linenum)
{
	const size_t errmsg_size = MAX_PATH + 64u;
	TCHAR errmsg[errmsg_size];
	_stprintf_s(errmsg, errmsg_size, TEXT("FILE: %s LINE: %d"), filepath, linenum);
	::MessageBox(NULL, errmsg, TEXT("ERROR"), MB_OK | MB_ICONERROR);
}

VGImage vgextLoadImage(LPCTSTR fn)
{
	return vgext::ImageLoader::getInstance().loadImageFromFile(fn);
}