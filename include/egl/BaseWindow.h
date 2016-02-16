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

#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <list>
#include <string>
#include <map>
#include <thread>
#include <EGL/egl.h>
#include "Vector.h"

namespace GE {

class Instance;

class BaseWindow
{
public:
	BaseWindow( Instance* instance, const std::string& title, int width, int height, uint32_t flags );
	~BaseWindow();
	void SetNativeWindow( EGLNativeWindowType win );

	uint32_t width();
	uint32_t height();
	Vector2i& cursor();
	Vector2i& cursorWarp();

	void WaitVSync( bool en );
	void SwapBuffersBase();
	float fps() const;

protected:
	Instance* mInstance;
	uint32_t mWidth;
	uint32_t mHeight;
	bool mHasResized;
	uint64_t mWindow;
	bool mKeys[512];
	Vector2i mCursor;
	Vector2i mCursorWarp;

	float mFps;
	int mFpsImages;
	uint64_t mFpsTimer;

	EGLDisplay mEGLDisplay;
	EGLConfig mEGLConfig;
	EGLContext mEGLContext;
};


} // namespace GE

#endif // BASEWINDOW_H
 