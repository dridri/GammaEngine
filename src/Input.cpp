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

#include <string.h>
#include "Input.h"

using namespace GE;

Input::Input( Window* window )
	: mWindow( window )
	, mCursor( Vector2i( 0, 0 ) )
	, mCursorLast( Vector2i( 0, 0 ) )
	, mCursorWarp( Vector2i( 0, 0 ) )
	, mKeys{ false }
{
}


Input::~Input()
{
}


void Input::Update()
{
	memcpy( mLastKeys, mKeys, sizeof( mKeys ) );
	mWindow->ReadKeys( mKeys );

	mCursorLast = mCursor;
	mCursor = mWindow->cursor();
	mCursorWarp = mWindow->cursorWarp();
}


bool Input::pressed( unsigned int keycode ) const
{
	return mKeys[ keycode ];
}


bool Input::toggled( unsigned int keycode ) const
{
	return mKeys[ keycode ] && !mLastKeys[ keycode ];
}


bool Input::untoggled( unsigned int keycode ) const
{
	return mLastKeys[ keycode ] && !mKeys[ keycode ];
}


Vector2f Input::cursor() const
{
	Vector2f ret;

	ret.x = (float)mCursor.x / mWindow->width();
	ret.y = (float)mCursor.y / mWindow->height();

	return ret;
}


Vector2f Input::cursorWarp() const
{
	Vector2f ret;

	ret.x = (float)mCursorWarp.x / mWindow->width();
	ret.y = (float)mCursorWarp.y / mWindow->height();

	return ret;
}
