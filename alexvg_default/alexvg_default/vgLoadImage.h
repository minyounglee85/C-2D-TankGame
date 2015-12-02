
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
	int width = bitmap.GetWidth();
	int height = bitmap.GetHeight();

	Gdiplus::BitmapData bitmapData;
	bitmap.LockBits( NULL, ImageLockModeRead, PixelFormat32bppARGB, &bitmapData );

	unsigned char* cimg = (unsigned char*)bitmapData.Scan0;
	unsigned int pitch = bitmapData.Stride;
	unsigned char* nimg = (unsigned char*)malloc( pitch*height );
	for( int y=0; y<height; y++ )
		for( int x=0; x<width; x++ )
			nimg[y*pitch+x*4+0] = cimg[(height-y-1)*pitch+x*4+3],	// A
			nimg[y*pitch+x*4+1] = cimg[(height-y-1)*pitch+x*4+0],	// B
			nimg[y*pitch+x*4+2] = cimg[(height-y-1)*pitch+x*4+1],	// G
			nimg[y*pitch+x*4+3] = cimg[(height-y-1)*pitch+x*4+2];	// R

	VGImage vgimage = vgCreateImage( VG_sRGBA_8888, width, height, VG_IMAGE_QUALITY_BETTER );
	vgImageSubData( vgimage, nimg, pitch, VG_sRGBA_8888, 0, 0, width, height );
	free( nimg );
	bitmap.UnlockBits(NULL);
	return vgimage;
}

#endif