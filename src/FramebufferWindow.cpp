/*
 * The GammaEngine Library 2.0 is a multiplatform Vulkan-based game engine
 * Copyright (C) 2015  Adrien Aubry <dridri85@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef GE_LINUX
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "FramebufferWindow.h"
#include "Window.h"
#include "Instance.h"
#include "include/Time.h"
#include "Input.h"
#include "Debug.h"

namespace GE {


FramebufferWindow::FramebufferWindow( Instance* instance, const std::string& title, int width, int height, uint32_t _flags )
	: mInstance( instance )
	, mWidth( width )
	, mHeight( height )
	, mHasResized( false )
	, mFramebuffer( nullptr )
	, mReversed( false )
	, mFramebufferAllocated( false )
{
	fDebug( instance, title, width, height, _flags );
	Window::Flags flags = static_cast<Window::Flags>( _flags );

	if ( width > 0 and height > 0 ) {
		mFramebuffer = (uint32_t*)instance->Malloc( sizeof( uint32_t ) * width * height );
		mBpp = 32;
		mFramebufferAllocated = true;
	}

	mWidth = width;
	mHeight = height;
}


FramebufferWindow::~FramebufferWindow()
{
	if ( mFramebufferAllocated ) {
		mInstance->Free( mFramebuffer );
	}
}


int FramebufferWindow::OpenSystemFramebuffer( const std::string& devfile, bool reverse, bool raw )
{
#ifdef GE_LINUX
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	int fbfd;
	void* fbp = nullptr;
	int bpp = 32;

	fbfd = open( devfile.c_str(), O_RDWR );
	if ( fbfd < 0 ) {
		gDebug() << "Unable to open display\n";
		return -1;
	}

	if ( not raw ) {
		if ( ioctl( fbfd, FBIOGET_FSCREENINFO, &finfo ) ) {
			gDebug() << "Unable to get display information (finfo)\n";
			goto try_raw;
		}
		if ( ioctl( fbfd, FBIOGET_VSCREENINFO, &vinfo ) and mWidth <= 0 and mHeight <= 0 ) {
			gDebug() << "Unable to get display information (vinfo)\n";
			goto try_raw;
		}

		printf( "Second display is %d x %d %dbps\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );

		fbp = (void*)mmap( 0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0 );

		mWidth = vinfo.xres;
		mHeight = vinfo.yres;
		bpp = vinfo.bits_per_pixel;
	} else {
try_raw:
		fbp = mmap( 0, mWidth * mHeight * ( mBpp / 4 ), PROT_READ|PROT_WRITE, MAP_SHARED, fbfd, 0);
	}

	if ( fbp <= 0 ) {
		gDebug() << "Unable to create mamory mapping\n";
		close( fbfd );
		return -1;
	}

	mReversed = reverse;
	mSystemFbHandler = fbfd;
	MapMemory( fbp, bpp );
	return 0;
#endif

	return -1;
}


void FramebufferWindow::MapMemory( void* fb_pointer, int bpp )
{
	if ( mFramebufferAllocated and mFramebuffer ) {
		mFramebufferAllocated = false;
		mInstance->Free( mFramebuffer );
	}
	mFramebuffer = (uint32_t*)fb_pointer;
	mBpp = bpp;

	gDebug() << "mapped : " << mFramebuffer << ", " << mWidth << "x" << mHeight << "\n";
}


void FramebufferWindow::ClearRegion( uint32_t color, int x, int y, int w, int h )
{
	uint8_t* fb8 = (uint8_t*)mFramebuffer;
	uint16_t* fb16 = (uint16_t*)mFramebuffer;
	uint32_t color32 = ( color & 0xFF00FF00 ) | ( ( color << 16 ) & 0x00FF0000 ) | ( ( color >> 16 ) & 0x000000FF );
	uint32_t color24 = color & 0x00FFFFFF;
// 	uint16_t color16 = ( ( color & 0xF80000 ) >> 8 ) | ( ( color & 0xFC00 ) >> 5 ) | ( ( color & 0xF8 ) >> 3 );
	uint16_t color16 = ( ( color & 0xF80000 ) >> 19 ) | ( ( color & 0xFC00 ) >> 5 ) | ( ( color & 0xF8 ) << 8 );
	uint32_t ofs = y * mWidth + x;

	if ( x == 0 and y == 0 and w < 0 and h < 0 ) {
		switch ( mBpp )
		{
			case 16 :
				for ( uint32_t i = 0; i < mWidth * mHeight; i++ ) {
					fb16[i] = color16;
				}
				break;
			case 24 :
				for ( uint32_t i = 0; i < mWidth * mHeight; i++ ) {
					memcpy( &fb8[i*3], &color24, 3 );
				}
				break;
			case 32 :
			default :
				for ( uint32_t i = 0; i < mWidth * mHeight; i++ ) {
					mFramebuffer[i] = color;
				}
				break;
		}
	} else {
		if ( mReversed ) {
			x = mWidth - x - w;
			y = mHeight - y - h;
			ofs = y * mWidth + x;
		}
		switch ( mBpp )
		{
			case 16 :
				for ( uint32_t j = ofs; j < ofs + mWidth * h; j += mWidth ) {
					for ( uint32_t i = 0; i < (uint32_t)w; i++ ) {
						fb16[ j + i ] = color16;
					}
				}
				break;
			case 24 :
				for ( uint32_t j = ofs; j < ofs + mWidth * h; j += mWidth ) {
					for ( uint32_t i = 0; i < (uint32_t)w; i++ ) {
						memcpy( &fb8[(j+i)*3], &color24, 3 );
					}
				}
				break;
			case 32 :
			default :
				for ( uint32_t j = ofs; j < ofs + mWidth * h; j += mWidth ) {
					for ( uint32_t i = 0; i < (uint32_t)w; i++ ) {
						mFramebuffer[j+i] = color;
					}
				}
				break;
		}
	}
}


uint32_t* FramebufferWindow::framebuffer()
{
	return mFramebuffer;
}


bool FramebufferWindow::reversed()
{
	return mReversed;
}


uint32_t FramebufferWindow::bpp()
{
	return mBpp;
}


uint32_t FramebufferWindow::width()
{
	return mWidth;
}


uint32_t FramebufferWindow::height()
{
	return mHeight;
}


Vector2i& FramebufferWindow::cursor()
{
	return mCursor;
}


Vector2i& FramebufferWindow::cursorWarp()
{
	return mCursorWarp;
}


float FramebufferWindow::fps() const
{
	return mFps;
}


void FramebufferWindow::WaitVSync( bool en )
{
	// Nothing to do
	(void)en;
}


void FramebufferWindow::SwapBuffersBase()
{
	mFpsImages++;
	if ( Time::GetTick() - mFpsTimer > 500 ) {
		mFps = mFpsImages * 1000.0f / ( Time::GetTick() - mFpsTimer );
		mFpsTimer = Time::GetTick();
		mFpsImages = 0;
	}

	// Nothing TODO ?
}

} // namespace GE 
