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

#include <string>
#include "Vector.h"

#ifndef CALLBACK
	#define CALLBACK __stdcall
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

	void SwapBuffersBase();
	float fps() const;

protected:
	Instance* mInstance;
	uint32_t mWidth;
	uint32_t mHeight;
	bool mHasResized;
	uint64_t mWindow;
	uint64_t hInstance;
	bool mKeys[512];
	Vector2i mCursor;
	Vector2i mCursorWarp;

	float mFps;
	int mFpsImages;
	uint64_t mFpsTimer;

private:
	static uintptr_t CALLBACK sWindowProcedure( uint64_t window, uint64_t message, uint64_t wParam, uint64_t lParam );
	uintptr_t CALLBACK WindowProcedure( uint64_t window, uint64_t message, uint64_t wParam, uint64_t lParam );
	bool mInitializing;
};


} // namespace GE

#endif // BASEWINDOW_H
 