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

#define BASEWINDOW_CPP

#include "BaseWindow.h"
#include "Window.h"
#include "Instance.h"
#include "Time.h"
#include "Input.h"
#include "Debug.h"

#include <windows.h>
#undef CreateWindow

namespace GE {

BaseWindow::BaseWindow( Instance* instance, const std::string& title, int width, int height, uint32_t _flags )
	: mInstance( instance )
	, mWidth( width )
	, mHeight( height )
	, mHasResized( false )
	, mWindow( 0 )
	, mKeys{ false }
	, mInitializing( true )
{
	Window::Flags flags = static_cast<Window::Flags>( _flags );
	hInstance = (uint64_t)GetModuleHandle( nullptr );

	WNDCLASS winClass;
	RECT WindowRect;
	bool fullscreen = false;
	bool resizable = false;
	int nSamples = 1;
	if ( flags & Window::Fullscreen ) {
		fullscreen = true;
	}
	if ( flags & Window::Resizable ) {
		resizable = true;
	}

	int fsWidth = GetSystemMetrics( SM_CXSCREEN );
	int fsHeight = GetSystemMetrics( SM_CYSCREEN );

	if ( width < 0 ) {
		mWidth = fsWidth;
	}
	if ( height < 0 ) {
		mHeight = fsHeight;
	}
	WindowRect.left = 0;
	WindowRect.top = 0;
	WindowRect.right = mWidth;
	WindowRect.bottom = mHeight;

	memset( &winClass, 0x0, sizeof(winClass) );
	winClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    winClass.lpfnWndProc = (WNDPROC)BaseWindow::sWindowProcedure;
    winClass.cbClsExtra = 0;
    winClass.cbWndExtra = 0;
    winClass.hInstance = (HINSTANCE)hInstance;
	winClass.hIcon = LoadIcon( nullptr, IDI_APPLICATION );
	winClass.hCursor = LoadCursor( nullptr, IDC_ARROW );
	winClass.hbrBackground = (HBRUSH) GetStockObject( BLACK_BRUSH );
	winClass.lpszMenuName = nullptr;
    winClass.lpszClassName = "GammaEngine";
	RegisterClass( &winClass );

	int dwExStyle = 0;
	int dwStyle = 0;
/*
	if ( fullscreen ) {
		fsmode = true;
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth	= width;
		dmScreenSettings.dmPelsHeight	= height;
		dmScreenSettings.dmBitsPerPel	= 32;
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
	}
*/

	dwStyle = WS_OVERLAPPEDWINDOW;
	dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

	AdjustWindowRectEx( &WindowRect, dwStyle, FALSE, dwExStyle );

	gDebug() << "this base : " << (void*)this << "\n";
	mWindow = (uint64_t)CreateWindowEx( dwExStyle, "GammaEngine", title.c_str(), WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, WindowRect.right-WindowRect.left, WindowRect.bottom-WindowRect.top, NULL, NULL, (HINSTANCE)hInstance, this);

	static	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof (pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 2;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_STEREO;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	int fmt = ChoosePixelFormat( GetDC( (HWND)mWindow ), &pfd );
	SetPixelFormat( GetDC( (HWND)mWindow ), fmt, &pfd );
}


BaseWindow::~BaseWindow()
{
}


uint32_t BaseWindow::width()
{
	return mWidth;
}


uint32_t BaseWindow::height()
{
	return mHeight;
}


Vector2i& BaseWindow::cursor()
{
	return mCursor;
}


Vector2i& BaseWindow::cursorWarp()
{
	return mCursorWarp;
}


float BaseWindow::fps() const
{
	return mFps;
}


void BaseWindow::SwapBuffersBase()
{
	MSG message;
	POINT point;

	while ( PeekMessage( &message, NULL, 0, 0, PM_REMOVE ) ) {
		TranslateMessage( &message );
		DispatchMessage( &message );
	}

	GetCursorPos( &point );
	ScreenToClient( (HWND)mWindow, &point );
	mCursorWarp.x = point.x - mCursor.x;
	mCursorWarp.y = point.y - mCursor.y;
	mCursor.x = point.x;
	mCursor.y = point.y;

	mFpsImages++;
	if ( Time::GetTick() - mFpsTimer > 500 ) {
		mFps = mFpsImages * 1000.0f / ( Time::GetTick() - mFpsTimer );
		mFpsTimer = Time::GetTick();
		mFpsImages = 0;
	}

	mInitializing = false;
}


uintptr_t CALLBACK BaseWindow::sWindowProcedure( uint64_t window, uint64_t message, uint64_t wParam, uint64_t lParam )
{
	if ( message == WM_NCCREATE ) {
		BaseWindow* win = (BaseWindow*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLongPtr( (HWND)window, GWLP_USERDATA, (uintptr_t)win );
//		gDebug() << "set Ptr\n";
	}

	BaseWindow* thiz = (BaseWindow*)GetWindowLongPtr( (HWND)window, GWLP_USERDATA );
//	gDebug() << "thiz : " << (void*)thiz << "\n";
	if ( thiz == nullptr ) {
		return DefWindowProc( (HWND)window, message, wParam, lParam );
	}
	return thiz->WindowProcedure( window, message, wParam, lParam);
}

static int create_counter = 0;
uintptr_t CALLBACK BaseWindow::WindowProcedure( uint64_t _window, uint64_t _message, uint64_t _wParam, uint64_t _lParam )
{
	int key = 0;
//	fDebug( _window, _message, _wParam, _lParam );
//	gDebug() << "this : " << (void*)this << "\n";

	HWND window = (HWND)_window;
	UINT message = _message;
	WPARAM wParam = _wParam;
	LPARAM lParam = _lParam;
/*
	if(message == WM_SETFOCUS){
		has_focus = true;
		ShowCursor(cursor_visible);
	}
	if(message == WM_KILLFOCUS){
		has_focus = false;
		ShowCursor(true);
	}
*/

	if ( message == WM_DESTROY && !mInitializing ) {
		PostQuitMessage(0);
		exit(0);
	}


	if ( !mInitializing && message == WM_SIZE ) {
		if ( lParam != 0 ) {
				mWidth = (lParam & 0x0000FFFF);
				mHeight = ((lParam & 0xFFFF0000) >> 16);
				mHasResized = true;
		}
	}

//	if ( !mInitializing && has_focus )
	{
		switch ( message ) {
			case WM_CREATE:
				break;

			case WM_COMMAND:
				break;
/*
			case WM_CHAR:
				last_key = wParam;
				break;
*/
			case WM_KEYDOWN:
//				last_key = wParam;
				key = wParam;
				if ( key >= 'a' && key <= 'z' ) {
					key += ( 'A' - 'a' );
				}
				if ( key < 256 ) {
					mKeys[key] = true;
				} else {
					// TODO
				}
				break;

			case WM_KEYUP:
//				last_key = 0;
				key = wParam;
				if ( key >= 'a' && key <= 'z' ) {
					key += ( 'A' - 'a' );
				}
				if ( key < 256 ) {
					mKeys[key] = false;
				} else {
					// TODO
				}
				break;

			case WM_LBUTTONDOWN:
				mKeys[Input::LBUTTON] = true;
				break;

			case WM_LBUTTONUP:
				mKeys[Input::LBUTTON] = false;
				break;

			case WM_RBUTTONDOWN:
				mKeys[Input::RBUTTON] = true;
				break;

			case WM_RBUTTONUP:
				mKeys[Input::RBUTTON] = false;
				break;

			case WM_MBUTTONDOWN:
				mKeys[Input::MBUTTON] = true;
				break;

			case WM_MBUTTONUP:
				mKeys[Input::MBUTTON] = false;
				break;
/*
			case WM_MOUSEWHEEL:
				if(((short)((wParam & 0xFFFF0000) >> 16)) > 0){
					keys_pressed[GEK_MWHEELUP] = true;
				}
				if(((short)((wParam & 0xFFFF0000) >> 16)) < 0){
					keys_pressed[GEK_MWHEELDOWN] = true;
				}
				break;
*/
			case WM_MOUSEMOVE:
				break;

			case 0x00FF: //WM_INPUT
				break;
		}
	}

	return DefWindowProc( window, message, wParam, lParam );
}

} // namespace GE 
