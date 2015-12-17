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

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/extensions/Xrender.h>
#include <GL/glx.h>
#include <string.h>

#include "linux/BaseWindow.h"
#include "Window.h"
#include "Instance.h"
#include "Time.h"
#include "Input.h"
#include "gememory.h"


namespace GE {


BaseWindow::BaseWindow( Instance* instance, const std::string& title, int width, int height, uint32_t _flags )
	: mInstance( instance )
	, mWidth( width )
	, mHeight( height )
	, mHasResized( false )
	, mWindow( 0 )
	, mKeys{ false }
	, mFps( 0.0f )
	, mFpsImages( 0 )
	, mFpsTimer( 0 )
	/*, mDisplay( 0 )*/
{
	XInitThreads();

	Window::Flags flags = static_cast<Window::Flags>( _flags );

	// TODO / TBD : XFree86 screen scaling ?
	if ( flags & Window::Fullscreen ) {
		width = -1;
		height = -1;
	}

	mDisplay = XOpenDisplay( 0 );
	mScreen = DefaultScreen( mDisplay );


	GLXFBConfig *fbconfigs;
	int numfbconfigs;
	static int VisData[] = {
		GLX_X_RENDERABLE, True,
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_DOUBLEBUFFER, True,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
// 		GLX_SAMPLE_BUFFERS, 1,
// 		GLX_SAMPLES, 1,
		None
    };
	fbconfigs = glXChooseFBConfig(mDisplay, mScreen, VisData, &numfbconfigs);
	for(int i = 0; i<numfbconfigs; i++) {
		XVisualInfo* visual = (XVisualInfo*)glXGetVisualFromFBConfig(mDisplay, fbconfigs[i]);
		if ( !visual) {
			continue;
		}

		XRenderPictFormat* pict_format = XRenderFindVisualFormat(mDisplay, visual->visual);
		if ( !pict_format ) {
			continue;
		}

		mFBConfig = fbconfigs[i];
		mVisualInfo = visual;
		break;
	}

	mColorMap = XCreateColormap( mDisplay, RootWindow( mDisplay, mScreen ), mVisualInfo->visual, AllocNone );

	mWindowAttributes = (XSetWindowAttributes*)mInstance->Malloc( sizeof(XSetWindowAttributes) );
	mWindowAttributes->colormap = mColorMap;
	mWindowAttributes->border_pixel = 0;
	mWindowAttributes->background_pixmap = None;

	XWindowAttributes attribs;
	XGetWindowAttributes( mDisplay, RootWindow( mDisplay, mScreen ), &attribs );
	if ( width < 0 ) {
		mWidth = attribs.width;
	}
	if ( height < 0 ) {
		mHeight = attribs.height;
	}

	XSizeHints* win_size_hints = XAllocSizeHints();
	win_size_hints->flags = PSize;
	if ( !( flags & Window::Resizable ) ) {
		win_size_hints->flags = PSize | PMinSize | PMaxSize;
		win_size_hints->min_width = mWidth;
		win_size_hints->min_height = mHeight;
		win_size_hints->max_width = mWidth;
		win_size_hints->max_height = mHeight;
		win_size_hints->base_width = mWidth;
		win_size_hints->base_height = mHeight;
	}

	mWindowAttributes->override_redirect = false;
	mWindowAttributes->event_mask = ExposureMask | PointerMotionMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask;
	mWindow = XCreateWindow( mDisplay, RootWindow( mDisplay, mVisualInfo->screen ), 0, 0, mWidth, mHeight, 0, mVisualInfo->depth, InputOutput, mVisualInfo->visual, CWBorderPixel | CWBackPixmap | CWColormap | CWEventMask | CWOverrideRedirect, mWindowAttributes );

	XSetStandardProperties( mDisplay, mWindow, title.c_str(), title.c_str(), None, nullptr, 0, nullptr );
	XMapRaised( mDisplay, mWindow );

	if ( flags & Window::Fullscreen ) {
// 		int unused = 0;
		Atom wm_fullscreen = XInternAtom( mDisplay, "_NET_WM_STATE_FULLSCREEN", true );
		XChangeProperty( mDisplay, mWindow, XInternAtom( mDisplay, "_NET_WM_STATE", true), XA_ATOM, 32, PropModeReplace, (unsigned char *)&wm_fullscreen, 1 );
// 		XGetGeometry( mDisplay, mWindow, (::Window*)&unused, &unused, &unused, &mWidth, &mHeight, (uint32_t*)&unused, (uint32_t*)&unused );
	}


	Atom wmDelete = XInternAtom( mDisplay, "WM_DELETE_WINDOW", true );
	XSetWMProtocols( mDisplay, mWindow, &wmDelete, 1 );

	XSetWMNormalHints( mDisplay, mWindow, win_size_hints );
	XSetWMHints( mDisplay, mWindow, XAllocWMHints() );
	XSelectInput( mDisplay, mWindow, mWindowAttributes->event_mask );

/*
	XColor black;
	static char bm_no_data[] = { 0 };
	Pixmap bm_no = XCreateBitmapFromData( mDisplay, mWindow, bm_no_data, 8, 8 );
	invisible_cursor = XCreatePixmapCursor( mDisplay, bm_no, bm_no, &black, &black, 0, 0 );
	if (bm_no!=None)XFreePixmap( mDisplay, bm_no);
*/

// 	mEventThread = new std::thread( &BaseWindow::pEventThread, this );

	Instance::setLocale( std::string( getenv( "LANG" ) ).substr( 0, 2 ) );
	Instance::setUserName( std::string( getenv( "USER" ) ) );
	Instance::setUserEmail( std::string( getenv( "USER" ) ) + "@no-mail.nope" );
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


void BaseWindow::WaitVSync( bool en )
{
	void (*glXSwapIntervalEXT)( Display*, GLXDrawable, int ) = ( void (*)( Display*, GLXDrawable, int ) )glXGetProcAddressARB( (const GLubyte*)"glXSwapIntervalEXT" );
	glXSwapIntervalEXT( mDisplay, mWindow, en );
}


void BaseWindow::SwapBuffersBase()
{
	pEventThread();

	int unused, mx, my;
	uint64_t unwin;
	if ( XQueryPointer( mDisplay, mWindow, &unwin, &unwin, &unused, &unused, &mx, &my, (uint32_t*)&unused ) ) {
		if ( false ) {
			mCursorWarp.x = mx - mWidth / 2;
			mCursorWarp.y = my - mHeight / 2;
			mCursor.x = mWidth / 2;
			mCursor.y = mHeight / 2;
			XWarpPointer( mDisplay, mWindow, mWindow, 0, 0, 0, 0, mCursor.x, mCursor.y );
		} else {
			mCursorWarp.x = mx - mCursor.x;
			mCursorWarp.y = my - mCursor.y;
			mCursor.x = mx;
			mCursor.y = my;
		}
	}

	if ( mWheelEvents.size() > 0 ) {
		XButtonEvent* ev = (XButtonEvent*)mWheelEvents.front();
		if ( ev->type >= 0 ) {
			if ( ev->button == 4 ) {
				mKeys[ Input::MWHEELUP ] = ( ev->type == ButtonPress );
			}
			if ( ev->button == 5 ) {
				mKeys[ Input::MWHEELDOWN ] = ( ev->type == ButtonPress );
			}
		}
		mWheelEvents.pop_front();
	}

	mFpsImages++;
	if ( Time::GetTick() - mFpsTimer > 500 ) {
		mFps = mFpsImages * 1000.0f / ( Time::GetTick() - mFpsTimer );
		mFpsTimer = Time::GetTick();
		mFpsImages = 0;
	}
}


void BaseWindow::pEventThread()
{
	uint32_t ticks = Time::GetTick();
	bool finished = false;
	int key = 0;
	XEvent event;
	mHasResized = false;

// 	while ( 1 )
	{
		while ( XPending( mDisplay ) ) {
			XNextEvent( mDisplay, &event );
// 			if(event.type)printf("event: %d\n", event.type);
			if ( ( event.type == ButtonPress || event.type == ButtonRelease ) && ( event.xbutton.button == 4 || event.xbutton.button == 5 ) ) {
				XButtonEvent* ev = new XButtonEvent;
				memcpy( ev, &event.xbutton, sizeof( XButtonEvent ) );
				mWheelEvents.push_back( ev );
			}
			switch ( event.type ) {
				case ClientMessage:
// 					if ( XGetAtomName( mDisplay, event.xclient.message_type) == "WM_PROTOCOLS" ) {
// 						finished = true;
// 					}
					if ( (Atom)event.xclient.data.l[0] == XInternAtom( mDisplay, "WM_DELETE_WINDOW", False ) ) {
						finished = true;
					}
					break;
				case ConfigureNotify:
					mHasResized = true;
// 					( ( mWidth != event.xconfigure.width ) or ( mHeight != event.xconfigure.height ) );
					mWidth = event.xconfigure.width;
					mHeight = event.xconfigure.height;
					break;
				case KeymapNotify:
					XRefreshKeyboardMapping( &event.xmapping );
					break;
				case KeyPress:
					key = (int)XLookupKeysym(&event.xkey, 0);
					if ( key >= 'a' && key <= 'z' ) {
						key += ( 'A' - 'a' );
					}
					if ( key < 256 ) {
// 						printf( "'%c' pressed\n", key);
						mKeys[key] = true;
					} else {
						// TODO
						if ( mKeys[ Input::LALT ] && mKeys[ Input::F4 ] ) {
							finished = true;
						}
					}
					break;
				case KeyRelease:
					key = (int)XLookupKeysym(&event.xkey, 0);
					if ( key >= 'a' && key <= 'z' ) {
						key += ( 'A' - 'a' );
					}
					if ( key < 256 ) {
// 						printf( "'%c' released\n", key);
						mKeys[key] = false;
					} else {
						// TODO
					}
					break;
				case ButtonPress:
					if ( event.xbutton.button == 4 ) { // wheel up
						// TODO
					} else if ( event.xbutton.button == 5 ) { // wheel down
						// TODO
					} else if ( event.xbutton.button == 1 ) {
						mKeys[ Input::LBUTTON ] = true;
					} else if ( event.xbutton.button == 2 ) {
						mKeys[ Input::MBUTTON ] = true;
					} else if ( event.xbutton.button == 3 ) {
						mKeys[ Input::RBUTTON ] = true;
					}
					break;
				case ButtonRelease:
					if ( event.xbutton.button == 4 ) { // wheel up
						// TODO
					} else if ( event.xbutton.button == 5 ) { // wheel down
						// TODO
					} else if ( event.xbutton.button == 1 ) {
						mKeys[ Input::LBUTTON ] = false;
					} else if ( event.xbutton.button == 2 ) {
						mKeys[ Input::MBUTTON ] = false;
					} else if ( event.xbutton.button == 3 ) {
						mKeys[ Input::RBUTTON ] = false;
					}
					break;
				default:
					break;
			}
		}
		if ( finished ) {
			mClosing = true;
			finished = false;
			exit(0); // TODO : cleaner way to exit
			return;
		}
// 		ticks = Time::WaitTick( 1000 / 120, ticks );
	}
}


} // namespace GE 
