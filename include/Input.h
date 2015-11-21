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

#ifndef GE_INPUT_H
#define GE_INPUT_H

#include "Vector.h"
#include "Window.h"

namespace GE
{

class Input
{
public:
	typedef enum {
		LBUTTON = 256,
		RBUTTON,
		MBUTTON,
		MWHEELUP,
		MWHEELDOWN,
		BACK,
		TAB,
		CLEAR,
		ENTER,
		SHIFT,
		LALT,
		RALT,
		CONTROL,
		MENU,
		PAUSE,
		ESCAPE,
		SPACE,
		PRIOR,
		NEXT,
		END,
		HOME,
		LEFT,
		UP,
		RIGHT,
		DOWN,
		SELECT,
		PRINT,
		SNAPSHOT,
		INSERT,
		DELETE,
		HELP,
		LSUPER,
		RSUPER,
		NUMPAD0,
		NUMPAD1,
		NUMPAD2,
		NUMPAD3,
		NUMPAD4,
		NUMPAD5,
		NUMPAD6,
		NUMPAD7,
		NUMPAD8,
		NUMPAD9,
		MULTIPLY,
		ADD,
		SEPARATOR,
		SUBTRACT,
		DECIMAL,
		DIVIDE,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		NUMLOCK,
		SCROLL,
	} Key;

	Input( Window* window );
	~Input();

	void Update();
	bool pressed( unsigned int keycode ) const;
	bool toggled( unsigned int keycode ) const;
	bool untoggled( unsigned int keycode ) const;
	Vector2f cursor() const;
	Vector2f cursorWarp() const;

protected:
	Window* mWindow;
	Vector2i mCursor;
	Vector2i mCursorLast;
	Vector2i mCursorWarp;
	bool mKeys[512];
	bool mLastKeys[512];
};


}

#endif // GE_INPUT_H
