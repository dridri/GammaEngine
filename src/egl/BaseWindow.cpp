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
#define GL_GLEXT_PROTOTYPES

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <string.h>

#include "egl/BaseWindow.h"
#include "Window.h"
#include "Debug.h"
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
{
	Window::Flags flags = static_cast<Window::Flags>( _flags );

	EGLint attribList[] =
	{
		EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
		EGL_SURFACE_TYPE,      EGL_WINDOW_BIT,
		EGL_RED_SIZE,          8,
		EGL_GREEN_SIZE,        8,
		EGL_BLUE_SIZE,         8,
		EGL_ALPHA_SIZE,        8,
	//	EGL_DEPTH_SIZE,        24,
		EGL_STENCIL_SIZE,      0,
		EGL_NONE
	};

	EGLint numConfigs;
	EGLint majorVersion;
	EGLint minorVersion;
	EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

	mEGLDisplay = eglGetDisplay( EGL_DEFAULT_DISPLAY );
	eglInitialize( mEGLDisplay, &majorVersion, &minorVersion );
	eglGetConfigs( mEGLDisplay, nullptr, 0, &numConfigs );
	eglChooseConfig( mEGLDisplay, attribList, &mEGLConfig, 1, &numConfigs );
	mEGLContext = eglCreateContext( mEGLDisplay, mEGLConfig, EGL_NO_CONTEXT, contextAttribs );

	Instance::setLocale( std::string( getenv( "LANG" ) ).substr( 0, 2 ) );
	Instance::setUserName( std::string( getenv( "USER" ) ) );
	Instance::setUserEmail( std::string( getenv( "USER" ) ) + "@no-mail.nope" );
}


void BaseWindow::SetNativeWindow( EGLNativeWindowType win )
{
	EGLSurface surface;
	const EGLint egl_surface_attribs[] = {
		EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
		EGL_NONE,
	};

	surface = eglCreateWindowSurface( mEGLDisplay, mEGLConfig, win, egl_surface_attribs );
	mWindow = reinterpret_cast< uintptr_t >( surface );

	eglQuerySurface( mEGLDisplay, surface, EGL_WIDTH, (EGLint*)&mWidth );
	eglQuerySurface( mEGLDisplay, surface, EGL_HEIGHT, (EGLint*)&mHeight );

	eglMakeCurrent( mEGLDisplay, surface, surface, mEGLContext );

	// The following block is already called in OpenGLES20Window::OpenGLES20Window(), but the context was still not up
	gDebug() << "OpenGL version : " << glGetString( GL_VERSION ) << "\n";
	glViewport( 0, 0, mWidth, mHeight );
	glEnable( GL_CULL_FACE );
	glFrontFace( GL_CCW );
	glCullFace( GL_BACK );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
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
}


void BaseWindow::SwapBuffersBase()
{
	eglSwapBuffers( mEGLDisplay, reinterpret_cast< EGLSurface >( mWindow ) );

	mFpsImages++;
	if ( Time::GetTick() - mFpsTimer > 500 ) {
		mFps = mFpsImages * 1000.0f / ( Time::GetTick() - mFpsTimer );
		mFpsTimer = Time::GetTick();
		mFpsImages = 0;
	}
}

} // namespace GE 
