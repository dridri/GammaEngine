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

#include <fstream>
#include <vector>
#include <locale>
#include <iomanip>

#include "Window.h"
#include "Instance.h"
#include "FbObject.h"
#include "FbInstance.h"
#include "FbRenderer2D.h"
#include "FbWindow.h"
#include "Object.h"
#include "Debug.h"
#include "File.h"
#include "Camera.h"
#include "Image.h"
#include "Font.h"

#undef DrawText
#define GLSL(shader)  #shader

extern "C" GE::Renderer2D* CreateRenderer2D( GE::Instance* instance, uint32_t width, uint32_t height ) {
	return new FbRenderer2D( instance, width, height );
}


FbRenderer2D::FbRenderer2D( Instance* instance, uint32_t width, uint32_t height )
	: Renderer2D()
	, FbRenderer( instance )
	, mWidth( width )
	, mHeight( height )
{
	mMatrixProjection->Orthogonal( 0.0, mWidth, mHeight, 0.0, -2049.0, 2049.0 );
	mMatrixView->Identity();

	Compute();
}


FbRenderer2D::~FbRenderer2D()
{
}


void FbRenderer2D::setDepthTestEnabled( bool en )
{
	FbRenderer::setDepthTestEnabled( en );
}


void FbRenderer2D::setBlendingEnabled (bool en )
{
	FbRenderer::setRenderMode( en );
}


Matrix* FbRenderer2D::projectionMatrix()
{
	return mMatrixProjection;
}


Matrix* FbRenderer2D::viewMatrix()
{
	return mMatrixView;
}


int FbRenderer2D::LoadVertexShader( const void* data, size_t size )
{
	return FbRenderer::LoadVertexShader( data, size );
}


int FbRenderer2D::LoadVertexShader( const std::string& file )
{
	return FbRenderer::LoadVertexShader( file );
}


int FbRenderer2D::LoadFragmentShader( const void* data, size_t size )
{
	return FbRenderer::LoadFragmentShader( data, size );
}


int FbRenderer2D::LoadFragmentShader( const std::string& file )
{
	return FbRenderer::LoadFragmentShader( file );
}


void FbRenderer2D::Compute()
{
}


void FbRenderer2D::Prerender()
{
	if ( mAssociatedWindow != nullptr && ( mAssociatedWindow->width() != mWidth || mAssociatedWindow->height() != mHeight ) ) {
		mWidth = mAssociatedWindow->width();
		mHeight = mAssociatedWindow->height();
		mMatrixProjection->Orthogonal( 0.0, mWidth, mHeight, 0.0, -2049.0, 2049.0 );
	}
}


static uint32_t geColorMix( uint32_t c0, uint32_t c1, uint8_t f )
{
	uint8_t rev = 255 - f;
	uint8_t r = (uint8_t)std::min( ( c0 & 0xFF ) * rev / 255 + ( c1 & 0xFF ) * f / 255, 255u );
	uint8_t g = (uint8_t)std::min( ( ( c0 >> 8 ) & 0xFF ) * rev / 255 + ( ( c1 >> 8 ) & 0xFF ) * f / 255, 255u );
	uint8_t b = (uint8_t)std::min( ( ( c0 >> 16 ) & 0xFF ) * rev / 255 + ( ( c1 >> 16 ) & 0xFF ) * f / 255, 255u );
	uint8_t a = (uint8_t)std::min( ( ( c0 >> 24 ) & 0xFF ) + ( ( c1 >> 24 ) & 0xFF ), 255u );
	return ( a << 24 ) | ( b << 16 ) | ( g << 8 ) | r;
}


void FbRenderer2D::RenderFlat( int x, int y, Image* image, int tx, int ty, int tw, int th )
{
	int width = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->width();
	int height = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->height();
	uint8_t* fb = (uint8_t*)dynamic_cast< FbInstance*>( mInstance )->boundWindow()->framebuffer();
	bool reversed = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->reversed();
	uint32_t bpp = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->bpp();
	uint32_t byte_pp = bpp / 8;

	if ( reversed ) {
		fb += width * height * byte_pp;
		byte_pp = -byte_pp;
	}

	if ( x < 0 ) {
		tx += (-x);
		x = 0;
	}
	if ( y < 0 ) {
		ty += (-y);
		y = 0;
	}
	if ( x + tw >= width ) {
		tw = width - x;
	}
	if ( y + th >= height ) {
		th = height - y;
	}
	uint8_t* destinationData = fb + ( width * y + x ) * byte_pp;
	int destinationSkipX = byte_pp * ( width - tw );
	uint32_t* sourceData = &image->data()[ image->width() * ty + tx ];
	int sourceSkipX = image->width() - tw;
	int X, Y;
	uint8_t a = 0xFF;
	for ( Y = 0; Y < th; Y++, destinationData += destinationSkipX, sourceData += sourceSkipX ) {
		for ( X = 0; X < tw; X++, destinationData+=byte_pp, sourceData++ ) {
			uint32_t color = *sourceData;
			a = color >> 24;
			if (a >= 0x7E) {
				switch ( bpp ) {
					case 16 : {
//						uint16_t color16 = ( ( color & 0xF80000 ) >> 8 ) | ( ( color & 0xFC00 ) >> 5 ) | ( ( color & 0xF8 ) >> 3 );
						uint16_t color16 = ( ( color & 0xF80000 ) >> 19 ) | ( ( color & 0xFC00 ) >> 5 ) | ( ( color & 0xF8 ) << 8 );
							*((uint16_t*)destinationData) = color16;
						break;
					}
					default : {
						uint32_t color32 = ( color & 0xFF00FF00 ) | ( ( color << 16 ) & 0x00FF0000 ) | ( ( color >> 16 ) & 0x000000FF );
						if(a >= 0x7E) *((uint32_t*)destinationData) = color32;
//						*destinationData = geColorMix( *destinationData, color, a ); // TODO : add Renderer2D parameter to set alpha quality
						break;
					}
				}
			}
		}
	}
}


void FbRenderer2D::Render( Image* image, const Vertex2D* vertices, int n, const Matrix& matrix )
{
	uint32_t* fb = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->framebuffer();
	uint32_t bpp = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->bpp();
}


void FbRenderer2D::Draw( int x, int y, Image* image, int tx, int ty, int tw, int th, float angle )
{
	Prerender();

	if ( tw == -1 ) {
		tw = image->width();
	}
	if ( th == -1 ) {
		th = image->height();
	}

	if ( angle == 0.0f ) {
		RenderFlat( x, y, image, tx, ty, tw, th );
		return;
	}

	image->serverReference( mInstance );
	const float rx = 1.0f / image->width();
	const float ry = 1.0f / image->height();

	Vertex2D vertices[6];
	memset( vertices, 0, sizeof(vertices) );
	vertices[0].u = (float)tx * rx;
	vertices[0].v = (float)ty * ry;
	vertices[0].x = x;
	vertices[0].y = y;

	vertices[1].u = (float)(tx+tw) * rx;
	vertices[1].v = (float)(ty+th) * ry;
	vertices[1].x = x + image->width();
	vertices[1].y = y + image->height();

	vertices[2].u = (float)(tx+tw) * rx;
	vertices[2].v = (float)ty * ry;
	vertices[2].x = x + image->width();
	vertices[2].y = y;

	vertices[3].u = (float)tx * rx;
	vertices[3].v = (float)ty * ry;
	vertices[3].x = x;
	vertices[3].y = y;

	vertices[4].u = (float)tx * rx;
	vertices[4].v = (float)(ty+th) * ry;
	vertices[4].x = x;
	vertices[4].y = y + image->height();

	vertices[5].u = (float)(tx+tw) * rx;
	vertices[5].v = (float)(ty+th) * ry;
	vertices[5].x = x + image->width();
	vertices[5].y = y + image->height();

	vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = vertices[5].color = image->color();

	Matrix m;
	if ( angle != 0.0f ) {
		m.Translate( x + image->width() / 2, y + image->height() / 2, 0.0f );
		m.RotateZ( -angle );
		m.Translate( -x - image->width() / 2, -y - image->height() / 2, 0.0f );
	}

	Render( image, vertices, 6, m );
}


void FbRenderer2D::Draw( int x, int y, int w, int h, Image* image, int tx, int ty, int tw, int th, float angle )
{
	Prerender();

	if ( tw == -1 ) {
		tw = image->width();
	}
	if ( th == -1 ) {
		th = image->height();
	}

	if ( angle == 0.0f ) {
		// TODO
// 		RenderFlat( x, y, image, tx, ty, tw, th );
		return;
	}

	image->serverReference( mInstance );
	const float rx = 1.0f / image->width();
	const float ry = 1.0f / image->height();

	Vertex2D vertices[6];
	memset( vertices, 0, sizeof(vertices) );
	vertices[0].u = (float)tx * rx;
	vertices[0].v = (float)ty * ry;
	vertices[0].x = x;
	vertices[0].y = y;

	vertices[1].u = (float)(tx+tw) * rx;
	vertices[1].v = (float)(ty+th) * ry;
	vertices[1].x = x + w;
	vertices[1].y = y + h;

	vertices[2].u = (float)(tx+tw) * rx;
	vertices[2].v = (float)ty * ry;
	vertices[2].x = x + w;
	vertices[2].y = y;

	vertices[3].u = (float)tx * rx;
	vertices[3].v = (float)ty * ry;
	vertices[3].x = x;
	vertices[3].y = y;

	vertices[4].u = (float)tx * rx;
	vertices[4].v = (float)(ty+th) * ry;
	vertices[4].x = x;
	vertices[4].y = y + h;

	vertices[5].u = (float)(tx+tw) * rx;
	vertices[5].v = (float)(ty+th) * ry;
	vertices[5].x = x + w;
	vertices[5].y = y + h;

	vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = vertices[4].color = vertices[5].color = image->color();

	Matrix m;
	if ( angle != 0.0f ) {
		m.Translate( x + w / 2, y + h / 2, 0.0f );
		m.RotateZ( -angle );
		m.Translate( -x - w / 2, -y - h / 2, 0.0f );
	}

	Render( image, vertices, 6, m );
}


void FbRenderer2D::DrawText( int x, int y, Font* font, uint32_t color, const std::string& text, Renderer2D::TextFlags flags )
{
	int width = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->width();
	int height = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->height();
	uint32_t* fb = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->framebuffer();
	bool reversed = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->reversed();
	uint32_t bpp = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->bpp();
	Prerender();

	y += font->size();
	const int base_x = x;
	const char* data = text.data();
	const uint32_t len = text.length();

	uint32_t* lines_ofs = nullptr;
	uint32_t line = 0;

	if ( flags & Renderer2D::HCenter ) {
		uint32_t width = 0;
		for ( uint32_t i = 0; i < len; i++ ) {
			if ( data[i] == '\n' ) {
				line++;
			}
		}
		lines_ofs = new uint32_t[ ++line ];
		line = 0;
		for ( uint32_t i = 0; i < len; i++ ) {
			int _c = data[i];
			width += font->glyph( _c )->advX;

			if ( data[i] == '\n' ) {
				lines_ofs[line] = x - width / 2;
				line++;
				width = 0;
			}
		}
		lines_ofs[line] = x - width / 2;
		line = 0;
		x = lines_ofs[0];
	}

	for ( uint32_t i = 0; i < len; i++ ) {
		int _c = data[i];
		uint8_t c = _c;

		if ( data[i] == '\n' ) {
			x = lines_ofs ? lines_ofs[++line] : base_x;
			y += font->size();
			continue;
		}

		font->RenderCharacter( c, color, fb, x, y, width, height, bpp, reversed );
		x += font->glyph(c)->advX;
	}

	if ( lines_ofs ) {
		delete lines_ofs;
	}
}


void FbRenderer2D::DrawText( int x, int y, Font* font, uint32_t color, const std::wstring& text, Renderer2D::TextFlags flags )
{
	int width = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->width();
	int height = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->height();
	uint32_t* fb = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->framebuffer();
	bool reversed = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->reversed();
	uint32_t bpp = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->bpp();
	Prerender();

	y += font->size();
	const int base_x = x;
	const wchar_t* data = text.data();
	const uint32_t len = text.length();
	bool rerender = false;
	std::map< wchar_t, Font::Glyph >& glyphs = font->glyphs();

	for ( uint32_t i = 0; i < len; i++ ) {
		wchar_t c = data[i];
		if ( glyphs.count( c ) <= 0 ) {
			Font::Glyph g = {
				.c = c,
				.x = 0,
				.y = 0,
				.w = 0,
				.h = 0,
				.advX = 0,
				.posY = 0
			};
			glyphs.insert( std::make_pair( c, g ) );
			rerender = true;
		}
	}
	if ( rerender ) {
		font->RenderGlyphs();
	}

	uint32_t* lines_ofs = nullptr;
	uint32_t line = 0;

	if ( flags & Renderer2D::HCenter ) {
		uint32_t width = 0;
		for ( uint32_t i = 0; i < len; i++ ) {
			if ( data[i] == '\n' ) {
				line++;
			}
		}
		lines_ofs = new uint32_t[ ++line ];
		line = 0;
		for ( uint32_t i = 0; i < len; i++ ) {
			int _c = data[i];
			width += font->glyph( _c )->advX;

			if ( data[i] == '\n' ) {
				lines_ofs[line] = x - width / 2;
				line++;
				width = 0;
			}
		}
		lines_ofs[line] = x - width / 2;
		line = 0;
		x = lines_ofs[0];
	}

	for ( uint32_t i = 0; i < len; i++ ) {
		wchar_t c = data[i];

		if ( data[i] == '\n' ) {
			x = lines_ofs ? lines_ofs[++line] : base_x;
			y += font->size();
			continue;
		}

		font->RenderCharacter( c, color, fb, x, y, width, height, bpp, reversed );
		x += font->glyph(c)->advX;
	}

	if ( lines_ofs ) {
		delete lines_ofs;
	}
}


void FbRenderer2D::DrawLine( int x0, int y0, uint32_t color0, int x1, int y1, uint32_t color1 )
{
	// TODO : color stepping

	uint32_t color = color0;
	int width = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->width();
	int height = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->height();
	uint8_t* fb = (uint8_t*)dynamic_cast< FbInstance*>( mInstance )->boundWindow()->framebuffer();
	uint32_t bpp = dynamic_cast< FbInstance*>( mInstance )->boundWindow()->bpp();

	if ( dynamic_cast< FbInstance*>( mInstance )->boundWindow()->reversed() ) {
		x0 = width - x0;
		y0 = height - y0;
		x1 = width - x1;
		y1 = height - y1;
	}

	short int dy = y1 - y0;
	short int dx = x1 - x0;
	short int stepx, stepy;
	
	if (dy < 0) { dy = -dy;  stepy = -width; } else { stepy = width; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;
	dx <<= 1;

	y0 *= width;
	y1 *= width;
/*FIXME
	if((x0 >= 0) && (x0 < width) && ((y0/width) >= 0) && ((y0/width) < height)){
		switch ( bpp ) {
			case 16 : {
				uint16_t color16 = ( ( color & 0xF80000 ) >> 19 ) | ( ( color & 0xFC00 ) >> 5 ) | ( ( color & 0xF8 ) << 8 );
				((uint16_t*)fb)[x0+y0] = color16;
				break;
			}
			default : {
				uint32_t color32 = ( color & 0xFF00FF00 ) | ( ( color << 16 ) & 0x00FF0000 ) | ( ( color >> 16 ) & 0x000000FF );
				((uint32_t*)fb)[x0+y0] = color32;
				break;
			}
		}
	}
*/
	int fraction;
	if (dx > dy) {
		fraction = dy - (dx >> 1);
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx;
			}
			x0 += stepx;
			fraction += dy;
			if((x0>=0) && (x0<width) && ((y0/width)>=0) && ((y0/width)<=height)){
				switch ( bpp ) {
					case 16 : {
						uint16_t color16 = ( ( color & 0xF80000 ) >> 19 ) | ( ( color & 0xFC00 ) >> 5 ) | ( ( color & 0xF8 ) << 8 );
						((uint16_t*)fb)[x0+y0] = color16;
						break;
					}
					default : {
						uint32_t color32 = ( color & 0xFF00FF00 ) | ( ( color << 16 ) & 0x00FF0000 ) | ( ( color >> 16 ) & 0x000000FF );
						((uint32_t*)fb)[x0+y0] = color32;
						break;
					}
				}
			}else if(x1 < 0){
				break;
			}
		}
	} else {
		fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			if((x0>=0) && (x0<width) && ((y0/width)>=0) && ((y0/width)<=height)){
// 				fb[x0+y0] = color0;
				switch ( bpp ) {
					case 16 : {
						uint16_t color16 = ( ( color & 0xF80000 ) >> 19 ) | ( ( color & 0xFC00 ) >> 5 ) | ( ( color & 0xF8 ) << 8 );
						((uint16_t*)fb)[x0+y0] = color16;
						break;
					}
					default : {
						uint32_t color32 = ( color & 0xFF00FF00 ) | ( ( color << 16 ) & 0x00FF0000 ) | ( ( color >> 16 ) & 0x000000FF );
						((uint32_t*)fb)[x0+y0] = color32;
						break;
					}
				}
			}else if(y1 < 0){
				break;
			}
		}
	}
}


uintptr_t FbRenderer2D::attributeID( const std::string& name )
{
	return 0;
}


uintptr_t FbRenderer2D::uniformID( const std::string& name )
{
	return 0;
}


void FbRenderer2D::uniformUpload( const uintptr_t id, const float f )
{
}


void FbRenderer2D::uniformUpload( const uintptr_t id, const Vector2f& v )
{
}


void FbRenderer2D::uniformUpload( const uintptr_t id, const Vector3f& v )
{
}


void FbRenderer2D::uniformUpload( const uintptr_t id, const Vector4f& v )
{
}


void FbRenderer2D::uniformUpload( const uintptr_t id, const Matrix& v )
{
}
