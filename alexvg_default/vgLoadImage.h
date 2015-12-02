
#ifndef __vgLoadImage__
#undef WIN32_LEAN_AND_MEAN
#include "windows.h"
#pragma comment(lib, "gdiplus.lib")
#include "gdiplus.h"
using namespace Gdiplus;

#define MAX_LOADSTRING 100
#include "egl/egl.h"
#include "vg/openvg.h"
#include "vg/vgu.h"

inline VGImage vgLoadImage( const WCHAR* fn )
{
	static int gdiplus_initialized =0;
	if( !gdiplus_initialized )
	{
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR           gdiplusToken;
   
		// Initialize GDI+.
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);	
	}
	Gdiplus::Bitmap bitmap( fn );
	int width = bitmap.vgCreateImage( VG_sRGBA_8888, width, height, VG_IMAGE_QUALITY_BETTER );
	vgImageSubData( vgimage, nimg, pitch, VG_sRGBA_8888, 0, 0, width, height );
	free( nimg );
	bitmap.UnlockBits(NULL);
	return vgimage;
}

#endif