#ifndef __LoadImage_vcl__
#undef WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(lib, "gdiplus.lib")
#include <gdiplus.h>

#define MAX_LOADSTRING 100
#include "EGL/egl.h"
#include "VG/openvg.h"
#include "VG/vgu.h"

namespace vgext
{
	class ImageLoader {
	private:
		ImageLoader();

	public:
		~ImageLoader();
		static ImageLoader & getInstance();
		VGImage loadImageFromFile(LPCTSTR fn); 

	private:
		static ImageLoader instance;
		Gdiplus::GdiplusStartupInput	gdiplusStartupInput;
		ULONG_PTR						gdiplusToken;
	};
}

#define VGEXTPRINTERROR() vgextPrintError(TEXT(__FILE__), (__LINE__))
void vgextPrintError(LPCTSTR filepath, int linenum);
VGImage vgextLoadImage(LPCTSTR fn);

#endif