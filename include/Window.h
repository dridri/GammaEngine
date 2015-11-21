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

#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include "BaseWindow.h"

namespace GE {

class Instance;

class Window : public BaseWindow
{
public:
	typedef enum {
		Nil = 0,
		Resizable = 1,
		Fullscreen = 2,
	} Flags;
	Window( Instance* instance, const std::string& title, int width, int height, Flags flags = Nil ) : BaseWindow( instance, title, width, height, (uint32_t)flags ) {}
	virtual ~Window(){};

	virtual void Clear( uint32_t color = 0xFF000000 ) = 0;
	virtual void BindTarget() = 0;
	virtual void SwapBuffers() = 0;

	virtual uint64_t colorImage() = 0;

	virtual void ReadKeys( bool* keys ) = 0;
	virtual uint64_t CreateSharedContext() = 0;
	virtual void BindSharedContext( uint64_t ctx ) = 0;
};


} // namespace GE

#endif // WINDOW_H
 