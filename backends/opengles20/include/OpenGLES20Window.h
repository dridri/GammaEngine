/*
 * The GammaEngine Library 2.0 is a multiplatform -based game engine
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

#ifndef OPENGLES20WINDOW_H
#define OPENGLES20WINDOW_H

#include <string>

#include "Window.h"
#include "BaseWindow.h"

namespace GE {
	class Instance;
};

using namespace GE;

class OpenGLES20Window : public GE::Window
{
public:
	OpenGLES20Window( Instance* instance, const std::string& title, int width, int height, Flags flags = Nil );
	~OpenGLES20Window();

	virtual void Clear( uint32_t color = 0xFF000000 );
	virtual void BindTarget();
	virtual void SwapBuffers();

	virtual uint64_t colorImage();

	virtual void ReadKeys( bool* keys );
	virtual uint64_t CreateSharedContext();
	virtual void BindSharedContext( uint64_t ctx );

private:
	void* mEGLDisplay;
	void* mEGLSurface;
	void* mEGLContext;
	uint32_t mClearColor;
};

extern "C" {
#ifdef _WIN32
// #include <GLES2/gl2.h>
// #include <GLES2/gl2ext.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/wglext.h>
#ifdef BASEWINDOW_CPP
	#define define_proc(base, name) PFNGL##base##PROC name
#else
	#define define_proc(base, name) extern PFNGL##base##PROC name
#endif
define_proc(ACTIVETEXTURE, glActiveTexture);
define_proc(ENABLEVERTEXATTRIBARRAY, glEnableVertexAttribArray);
define_proc(DISABLEVERTEXATTRIBARRAY, glDisableVertexAttribArray);
define_proc(VERTEXATTRIBPOINTER, glVertexAttribPointer);
define_proc(GENBUFFERS, glGenBuffers);
define_proc(DELETEBUFFERS, glDeleteBuffers);
define_proc(BINDBUFFER, glBindBuffer);
define_proc(BUFFERDATA, glBufferData);
define_proc(BUFFERSUBDATA, glBufferSubData);
define_proc(GETBUFFERPARAMETERIV, glGetBufferParameteriv);
define_proc(RENDERBUFFERSTORAGE, glRenderbufferStorage);
define_proc(GENRENDERBUFFERS, glGenRenderbuffers);
define_proc(BINDRENDERBUFFER, glBindRenderbuffer);
define_proc(FRAMEBUFFERRENDERBUFFER, glFramebufferRenderbuffer);
define_proc(GENFRAMEBUFFERS, glGenFramebuffers);
define_proc(DELETEFRAMEBUFFERS, glDeleteFramebuffers);
define_proc(BINDFRAMEBUFFER, glBindFramebuffer);
define_proc(CREATESHADER, glCreateShader);
define_proc(SHADERSOURCE, glShaderSource);
define_proc(COMPILESHADER, glCompileShader);
define_proc(ATTACHSHADER, glAttachShader);
define_proc(GETSHADERINFOLOG, glGetShaderInfoLog);
define_proc(DELETESHADER, glDeleteShader);
define_proc(DELETEPROGRAM, glDeleteProgram);
define_proc(CREATEPROGRAM, glCreateProgram);
define_proc(LINKPROGRAM, glLinkProgram);
define_proc(USEPROGRAM, glUseProgram);
define_proc(GETPROGRAMINFOLOG, glGetProgramInfoLog);
define_proc(BINDATTRIBLOCATION, glBindAttribLocation);
define_proc(GETUNIFORMLOCATION, glGetUniformLocation);
define_proc(GETATTRIBLOCATION, glGetAttribLocation);
define_proc(UNIFORM1I, glUniform1i);
define_proc(UNIFORM2I, glUniform2i);
define_proc(UNIFORM3I, glUniform3i);
define_proc(UNIFORM4I, glUniform4i);
define_proc(UNIFORM1F, glUniform1f);
define_proc(UNIFORM2F, glUniform2f);
define_proc(UNIFORM3F, glUniform3f);
define_proc(UNIFORM4F, glUniform4f);
define_proc(UNIFORM1FV, glUniform1fv);
define_proc(UNIFORM2FV, glUniform2fv);
define_proc(UNIFORM3FV, glUniform3fv);
define_proc(UNIFORM4FV, glUniform4fv);
define_proc(UNIFORM1IV, glUniform1iv);
define_proc(UNIFORM2IV, glUniform2iv);
define_proc(UNIFORM3IV, glUniform3iv);
define_proc(UNIFORM4IV, glUniform4iv);
define_proc(UNIFORMMATRIX3FV, glUniformMatrix3fv);
define_proc(UNIFORMMATRIX4FV, glUniformMatrix4fv);
define_proc(GETUNIFORMFV, glGetUniformfv);
/*
define_proc(ENABLE, glEnable);
define_proc(DISABLE, glDisable);

define_proc(DRAWARRAYS, glDrawArrays);
define_proc(DRAWELEMENTS, glDrawElements);

define_proc(BLENDFUNC, glBlendFunc);

define_proc(GENTEXTURES, glGenTextures);
define_proc(DELETETEXTURES, glDeleteTextures);
define_proc(BINDTEXTURE, glBindTexture);
define_proc(TEXIMAGE2D, glTexImage2D);
define_proc(TEXPARAMETERF, glTexParameterf);

define_proc(GETSTRING, glGetString);
define_proc(CLEAR, glClear);
define_proc(CLEARCOLOR, glClearColor);
define_proc(VIEWPORT, glViewport);
define_proc(FRONTFACE, glFrontFace);
define_proc(CULLFACE, glCullFace);
*/

#endif
};

#endif // OPENGLES20WINDOW_H

