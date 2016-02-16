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

#ifndef FRAMEBUFFERRENDERER2D_H
#define FRAMEBUFFERRENDERER2D_H

#include <vector>
#include "FbRenderer.h"
#include "Renderer2D.h"
#include "Object.h"
#include "Image.h"
#include "Light.h"

class FbRenderer2D : public Renderer2D, public FbRenderer
{
public:
	FbRenderer2D( Instance* instance, uint32_t width, uint32_t height );
	~FbRenderer2D();

	virtual void setDepthTestEnabled( bool en );
	virtual void setBlendingEnabled( bool en );

	virtual Matrix* projectionMatrix();
	virtual Matrix* viewMatrix();

	virtual int LoadVertexShader( const std::string& file );
	virtual int LoadVertexShader( const void* data, size_t size );
	virtual int LoadFragmentShader( const std::string& file );
	virtual int LoadFragmentShader( const void* data, size_t size );

	virtual uintptr_t attributeID( const std::string& name );
	virtual uintptr_t uniformID( const std::string& name );
	virtual void uniformUpload( const uintptr_t id, const float f );
	virtual void uniformUpload( const uintptr_t id, const Vector2f& v );
	virtual void uniformUpload( const uintptr_t id, const Vector3f& v );
	virtual void uniformUpload( const uintptr_t id, const Vector4f& v );
	virtual void uniformUpload( const uintptr_t id, const Matrix& v );

	virtual void Draw( int x, int y, Image* image, int tx, int ty, int tw, int th, float angle );
	virtual void Draw( int x, int y, int w, int h, Image* image, int tx, int ty, int tw, int th, float angle );

	virtual void DrawText( int x, int y, Font* font, uint32_t color, const std::string& text, TextFlags flags = Nil );
	virtual void DrawText( int x, int y, Font* font, uint32_t color, const std::wstring& text, TextFlags flags = Nil );

	virtual void DrawLine( int x0, int y0, uint32_t color0, int x1, int y1, uint32_t color1 );

protected:
	void Compute();
	void Prerender();
	void RenderFlat( int x, int y, Image* image, int tx, int ty, int tw, int th );
	void Render( Image* image, const Vertex2D* vertices, int n, const Matrix& matrix );
	uint32_t mWidth;
	uint32_t mHeight;
};

#endif // FRAMEBUFFERRENDERER2D_H
