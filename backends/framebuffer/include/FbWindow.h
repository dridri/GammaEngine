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

#ifndef FBWINDOW_H
#define FBWINDOW_H

#include <string>

#include "Window.h"
#include "FramebufferWindow.h"

namespace GE {
	class Instance;
};

using namespace GE;

class FbWindow : public ProxyWindow< FramebufferWindow >
{
public:
	FbWindow( Instance* instance, const std::string& title, int width, int height, Window::Flags flags = Window::Nil );
	~FbWindow();

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

#endif // FBWINDOW_H

