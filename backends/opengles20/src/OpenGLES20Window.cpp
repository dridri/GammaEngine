/*
 * The GammaEngine Library 2.0 is a multiplatform Vulkan-based game engine
 * Copyright (C) 2015  Adrien Aubry <dridri85@gmail.com>
 *
 * This provkam is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This provkam is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this provkam.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define BASEWINDOW_CPP

#ifdef GE_WIN32
	static bool glext_loaded = false;
	static void load_glext();
	#include <windows.h>
	#undef CreateWindow
//	#include <GLES2/gl2.h>
//	#include <GLES2/gl2ext.h>
	#include <GL/gl.h>
	#include <GL/wglext.h>
#elif defined(GE_EGL)
	#include <EGL/egl.h>
	#include <EGL/eglext.h>
	#include <EGL/eglplatform.h>
#elif defined(GE_LINUX)
	#include <EGL/egl.h>
	#include <EGL/eglext.h>
	#include <EGL/eglplatform.h>
	#include <GL/glx.h>
	#include <X11/Xlib.h>
#endif


#if GE_IOS
void _ge_iOSFramebufferUseDefault();
#endif

#include "OpenGLES20Window.h"
#include "OpenGLES20Instance.h"
#include "Debug.h"

extern "C" GE::Window* CreateWindow( GE::Instance* instance, const std::string& title, int width, int height, OpenGLES20Window::Flags flags ) {
	return new OpenGLES20Window( instance, title, width, height, flags );
}

OpenGLES20Window::OpenGLES20Window( Instance* instance, const std::string& title, int width, int height, Flags flags )
	: GE::Window( instance, title, width, height, flags )
	, mClearColor( 0 )
{
	// TODO : MSAA
	int nSamples = 1;

	if ( flags & NoContext ) {
		gDebug() << "OpenGL version : " << glGetString( GL_VERSION ) << "\n";
	} else {
	#ifndef GE_QT
		#ifdef GE_WIN32

			mEGLContext = wglCreateContext( GetDC( (HWND)mWindow ) );
			wglMakeCurrent( GetDC( (HWND)mWindow ), (HGLRC)mEGLContext );

			ShowWindow( (HWND)mWindow, SW_SHOWNORMAL );

			if ( !glext_loaded ) {
				load_glext();
				glext_loaded = true;
			}

		#elif defined(GE_EGL)
		#elif defined(GE_LINUX)
			mEGLContext = glXCreateContext( mDisplay, (XVisualInfo*)mVisualInfo, 0, true );
			glXMakeCurrent( mDisplay, mWindow, static_cast<GLXContext>(mEGLContext) );
		#elif defined(GE_ANDROID)
		#elif defined(GE_IOS)

		#endif
	#endif

		gDebug() << "OpenGL version : " << glGetString( GL_VERSION ) << "\n";

		glViewport( 0, 0, mWidth, mHeight );
		glEnable( GL_CULL_FACE );
		glFrontFace( GL_CCW );
		glCullFace( GL_BACK );
		glEnable( GL_DEPTH_TEST );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	}
	((OpenGLES20Instance*)mInstance)->AffectVRAM( sizeof(uint32_t) * mWidth * mHeight ); // Front
	((OpenGLES20Instance*)mInstance)->AffectVRAM( sizeof(uint32_t) * mWidth * mHeight ); // Back
	((OpenGLES20Instance*)mInstance)->AffectVRAM( sizeof(uint32_t) * mWidth * mHeight / 2 ); // Depth
}


OpenGLES20Window::~OpenGLES20Window()
{
}


uint64_t OpenGLES20Window::colorImage()
{
	return 0;
}


void OpenGLES20Window::Clear( uint32_t color, bool colorBuffer, bool depthBuffer )
{
	BindTarget();

	if ( colorBuffer ) {
		float clearColor[] = {
			(float)( ( color >>  0 ) & 0xFF ) / 255.0f,
			(float)( ( color >>  8 ) & 0xFF ) / 255.0f,
			(float)( ( color >> 16 ) & 0xFF ) / 255.0f,
			(float)( ( color >> 24 ) & 0xFF ) / 255.0f
		};
		glClearColor( clearColor[0], clearColor[1], clearColor[2], clearColor[3] );
	}
	glClear( ( colorBuffer ? GL_COLOR_BUFFER_BIT : 0 ) | ( depthBuffer ? GL_DEPTH_BUFFER_BIT : 0 ) );
}


void OpenGLES20Window::BindTarget()
{
#if GE_IOS
	_ge_iOSFramebufferUseDefault();
#else
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
#endif
}


void OpenGLES20Window::SwapBuffers()
{
#ifndef GE_QT
	#ifdef GE_WIN32
		::SwapBuffers( GetDC( (HWND)mWindow ) );
	#elif defined(GE_EGL)
	#elif defined(GE_LINUX)
		glXSwapBuffers( mDisplay, mWindow );
	#elif defined(GE_ANDROID)
		glFinish();
	#endif
#endif
	SwapBuffersBase();

	if ( mHasResized ) {
		gDebug() << "has resized\n";
		glViewport( 0, 0, mWidth, mHeight );
	}
}


uint64_t OpenGLES20Window::CreateSharedContext()
{
#ifndef GE_QT
	#ifdef GE_WIN32
		return 0;
	#elif defined(GE_EGL)
		return 0;
	#elif defined(GE_LINUX)
		return (uint64_t)glXCreateContext( mDisplay, (XVisualInfo*)mVisualInfo, (GLXContext)mEGLContext, true );
	#elif defined(GE_EGL)
		return 0;
	#elif defined(GE_ANDROID)
		return 0;
	#elif defined(GE_IOS)
		return 0;
	#endif
#else
	return 0;
#endif
}


void OpenGLES20Window::BindSharedContext( uint64_t ctx )
{
#ifndef GE_QT
	#ifdef GE_WIN32
	#elif defined(GE_EGL)
	#elif defined(GE_LINUX)
		int ret = glXMakeCurrent( mDisplay, mWindow, static_cast<GLXContext>((void*)ctx) );
		gDebug() << "glXMakeCurrent returned : " << ret << "\n";
	#elif defined(GE_ANDROID)
	#elif defined(GE_IOS)
	#endif
#endif
}


void OpenGLES20Window::ReadKeys( bool* keys )
{
	memcpy( keys, mKeys, sizeof( mKeys ) );
}


#ifdef _WIN32
static void load_glext()
{
	#include <windows.h>
	#define load_func(name) name = (decltype(name))GetProcAddress(hOpenGL, #name); if ( !name ) { name = (decltype(name))wglGetProcAddress(#name); } printf("" #name " : 0x%p\n", name)
	HMODULE hOpenGL = LoadLibrary("opengl32.dll");
	load_func( glActiveTexture );
	load_func( glEnableVertexAttribArray );
	load_func( glDisableVertexAttribArray );
	load_func( glVertexAttribPointer );
	load_func( glGenBuffers );
	load_func( glDeleteBuffers );
	load_func( glBindBuffer );
	load_func( glBufferData );
	load_func( glBufferSubData );
	load_func( glGetBufferParameteriv );
	load_func( glGenRenderbuffers );
	load_func( glBindRenderbuffer );
	load_func( glFramebufferRenderbuffer );
	load_func( glGenFramebuffers );
	load_func( glDeleteFramebuffers );
	load_func( glBindFramebuffer );
	load_func( glCreateShader );
	load_func( glShaderSource );
	load_func( glCompileShader );
	load_func( glAttachShader );
	load_func( glGetShaderInfoLog );
	load_func( glDeleteShader );
	load_func( glDeleteProgram );
	load_func( glCreateProgram );
	load_func( glLinkProgram );
	load_func( glUseProgram );
	load_func( glGetProgramInfoLog );
	load_func( glBindAttribLocation );
	load_func( glGetUniformLocation );
	load_func( glGetAttribLocation );
	load_func( glUniform1i );
	load_func( glUniform2i );
	load_func( glUniform3i );
	load_func( glUniform4i );
	load_func( glUniform1f );
	load_func( glUniform2f );
	load_func( glUniform3f );
	load_func( glUniform4f );
	load_func( glUniform1fv );
	load_func( glUniform2fv );
	load_func( glUniform3fv );
	load_func( glUniform4fv );
	load_func( glUniform1iv );
	load_func( glUniform2iv );
	load_func( glUniform3iv );
	load_func( glUniform4iv );
	load_func( glUniformMatrix3fv );
	load_func( glUniformMatrix4fv );
	load_func( glGetUniformfv );
	load_func( glRenderbufferStorage );
/*
	load_func( glDrawArrays );
	load_func( glDrawElements );
	load_func( glBlendFunc );
	load_func( glBindTexture );
	load_func( glEnable );
	load_func( glDisable );
	load_func( glGenTextures );
	load_func( glDeleteTextures );
	load_func( glBindTexture );
	load_func( glTexImage2D );
	load_func( glTexParameterf );
	load_func( glGetString );
	load_func( glClear );
	load_func( glClearColor );
	load_func( glViewport );
	load_func( glFrontFace );
	load_func( glCullFace );
*/
}
#endif
