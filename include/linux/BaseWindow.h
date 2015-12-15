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
// #include <vulkan.h>
#include "Vector.h"

#ifndef BASEWINDOW_CPP
#ifndef Display
#define Display void
#endif
#ifndef XSetWindowAttributes
#define XSetWindowAttributes void
#endif
#ifndef GLXFBConfig
#define GLXFBConfig void*
#endif
#ifndef XVisualInfo
#define XVisualInfo void
#endif
#endif

namespace GE {

class Instance;

class BaseWindow
{
public:
	BaseWindow( Instance* instance, const std::string& title, int width, int height, uint32_t flags );
	~BaseWindow();

	uint32_t width();
	uint32_t height();
	Vector2i& cursor();
	Vector2i& cursorWarp();

	void WaitVSync( bool en );
	void SwapBuffersBase();
	float fps() const;

protected:
	void pEventThread();

	Instance* mInstance;
	uint32_t mWidth;
	uint32_t mHeight;
	bool mHasResized;
	uint64_t mWindow;
	std::thread* mEventThread;
	bool mKeys[512];
	Vector2i mCursor;
	Vector2i mCursorWarp;

	std::list< void* > mWheelEvents;

	float mFps;
	int mFpsImages;
	uint64_t mFpsTimer;

protected://private:
	Display* mDisplay;
	int mScreen;
	GLXFBConfig mFBConfig;
	XVisualInfo* mVisualInfo;
	uint64_t mColorMap;
	XSetWindowAttributes* mWindowAttributes;
	bool mClosing;
};


} // namespace GE

#endif // BASEWINDOW_H
 