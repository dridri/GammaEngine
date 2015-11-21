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

#ifndef RENDERER2D_H
#define RENDERER2D_H

#include <vector>

namespace GE {

class Instance;
class Object;
class Light;
class Camera;
class Matrix;
class Image;
class Font;
class Window;

class Renderer2D
{
public:
	typedef enum {
		Nil = 0,
		HCenter = 1,
	} TextFlags;

	Renderer2D() : mAssociatedWindow( nullptr ) {}
	virtual ~Renderer2D(){};

	virtual int LoadVertexShader( const std::string& file ) = 0;
	virtual int LoadVertexShader( const void* data, size_t size ) = 0;
	virtual int LoadFragmentShader( const std::string& file ) = 0;
	virtual int LoadFragmentShader( const void* data, size_t size ) = 0;

	virtual void Draw( int x, int y, Image* image, int tx = 0, int ty = 0, int tw = -1, int th = -1, float angle = 0.0f ) = 0;
	virtual void Draw( int x, int y, int w, int h, Image* image, int tx = 0, int ty = 0, int tw = -1, int th = -1, float angle = 0.0f ) = 0;

	virtual void DrawText( int x, int y, Font* font, uint32_t color, const std::string& text, TextFlags flags = Nil ) = 0;
	virtual void DrawText( int x, int y, Font* font, uint32_t color, const std::wstring& text, TextFlags flags = Nil ) = 0;

	virtual void DrawLine( int x0, int y0, uint32_t color0, int x1, int y1, uint32_t color1 ) = 0;

	void AssociateSize( Window* window ) { mAssociatedWindow = window; };

	virtual uintptr_t attributeID( const std::string& name ) = 0;
	virtual uintptr_t uniformID( const std::string& name ) = 0;
	virtual void uniformUpload( const uintptr_t id, const float f ) = 0;
	virtual void uniformUpload( const uintptr_t id, const Vector2f& v ) = 0;
	virtual void uniformUpload( const uintptr_t id, const Vector3f& v ) = 0;
	virtual void uniformUpload( const uintptr_t id, const Vector4f& v ) = 0;
	virtual void uniformUpload( const uintptr_t id, const Matrix& v ) = 0;

protected:
	Window* mAssociatedWindow;
};

} // namespace GE

#endif // RENDERER2D_H