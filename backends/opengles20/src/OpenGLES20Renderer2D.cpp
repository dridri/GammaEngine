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
#include "OpenGLES20Object.h"
#include "OpenGLES20Instance.h"
#include "OpenGLES20Renderer2D.h"
#include "OpenGLES20Window.h"
#include "Object.h"
#include "Debug.h"
#include "File.h"
#include "Camera.h"
#include "Image.h"
#include "Font.h"

#undef DrawText
#define GLSL(shader)  #shader

extern "C" GE::Renderer2D* CreateRenderer2D( GE::Instance* instance, uint32_t width, uint32_t height ) {
	return new OpenGLES20Renderer2D( instance, width, height );
}
static const char vertices_shader_base[] = GLSL(
	out vec4 ge_Color;
	out vec2 ge_TextureCoord;

	void main()
	{
		ge_Color = ge_VertexColor;
		ge_TextureCoord = ge_VertexTexcoord.xy;
		ge_Position = ge_ProjectionMatrix /* ge_ViewMatrix*/ * ge_ObjectMatrix * vec4(ge_VertexPosition.xy, 0.0, 1.0);
	}
);

static const char fragment_shader_base[] = GLSL(
	in vec4 ge_Color;
	in vec2 ge_TextureCoord;

	void main()
	{
		ge_FragColor = ge_Color * texture( ge_Texture0, ge_TextureCoord.xy );
/*		if ( ge_FragColor.a <= 0.2 ) {
			discard;
		}*/
	}
);


OpenGLES20Renderer2D::OpenGLES20Renderer2D( Instance* instance, uint32_t width, uint32_t height )
	: Renderer2D()
	, OpenGLES20Renderer( instance )
	, m2DReady( false )
	, mWidth( width )
	, mHeight( height )
{
	mMatrixProjection->Orthogonal( 0.0, mWidth, mHeight, 0.0, -2049.0, 2049.0 );
	mMatrixView->Identity();

	mTextureWhite = new Image( 1, 1, 0xFFFFFFFF, instance );

	OpenGLES20Renderer::LoadVertexShader( vertices_shader_base, sizeof(vertices_shader_base) + 1 );
	OpenGLES20Renderer::LoadFragmentShader( fragment_shader_base, sizeof(fragment_shader_base) + 1 );

	Compute();
}


OpenGLES20Renderer2D::~OpenGLES20Renderer2D()
{
}


int OpenGLES20Renderer2D::LoadVertexShader( const void* data, size_t size )
{
	m2DReady = false;
	return OpenGLES20Renderer::LoadVertexShader( data, size );
}


int OpenGLES20Renderer2D::LoadVertexShader( const std::string& file )
{
	m2DReady = false;
	return OpenGLES20Renderer::LoadVertexShader( file );
}


int OpenGLES20Renderer2D::LoadFragmentShader( const void* data, size_t size )
{
	m2DReady = false;
	return OpenGLES20Renderer::LoadFragmentShader( data, size );
}


int OpenGLES20Renderer2D::LoadFragmentShader( const std::string& file )
{
	m2DReady = false;
	return OpenGLES20Renderer::LoadFragmentShader( file );
}


void OpenGLES20Renderer2D::Compute()
{
	createPipeline();


	glUseProgram( mShader );
	glUniformMatrix4fv( mMatrixProjectionID, 1, GL_FALSE, mMatrixProjection->constData() );

	// Pre-allocate 32 quads + 1 static quad
	glGenBuffers( 1, &mVBO );
	glBindBuffer( GL_ARRAY_BUFFER, mVBO );
	glBufferData( GL_ARRAY_BUFFER, ( 32 + 1 ) * 6 * sizeof(Vertex2D), nullptr, GL_DYNAMIC_DRAW );
	((OpenGLES20Instance*)Instance::baseInstance())->AffectVRAM( sizeof(Vertex2D) * ( 32 + 1 ) * 6 );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	m2DReady = true;
}


void OpenGLES20Renderer2D::Prerender()
{
	if ( !m2DReady ) {
		Compute();
	}
	if ( mAssociatedWindow != nullptr && ( mAssociatedWindow->width() != mWidth || mAssociatedWindow->height() != mHeight ) ) {
		mWidth = mAssociatedWindow->width();
		mHeight = mAssociatedWindow->height();
		mMatrixProjection->Orthogonal( 0.0, mWidth, mHeight, 0.0, -2049.0, 2049.0 );
		glUseProgram( mShader );
		glUniformMatrix4fv( mMatrixProjectionID, 1, GL_FALSE, mMatrixProjection->constData() );
	}
}


void OpenGLES20Renderer2D::Render( Image* image, int mode, int start, int n, const Matrix& matrix )
{
	glUseProgram( mShader );

	if (! s2DActive ) {
		glActiveTexture( GL_TEXTURE0 );
		glEnable( GL_TEXTURE_2D );

//		glUniformMatrix4fv( mMatrixViewID, 1, GL_FALSE, mMatrixView->constData() );

		glEnableVertexAttribArray( 0 );
		glEnableVertexAttribArray( 1 );
		glEnableVertexAttribArray( 2 );
		glEnableVertexAttribArray( 3 );

		glDisable( GL_DEPTH_TEST );
	//	glDisable( GL_BLEND );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		s2DActive = true;
	}

	glUniform1f( mFloatTimeID, Time::GetSeconds() );
	glUniformMatrix4fv( mMatrixObjectID, 1, GL_FALSE, matrix.constData() );

	glBindTexture( GL_TEXTURE_2D, image->serverReference( mInstance ) );
	glVertexAttribPointer( 1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( Vertex2D ), (void*)( 0 ) );
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex2D ), (void*)( sizeof( uint32_t ) ) );
	glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex2D ), (void*)( sizeof( uint32_t ) + sizeof( float ) * 2 ) );

	glDrawArrays( mode, start, n );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glUseProgram( 0 );
}


void OpenGLES20Renderer2D::Draw( int x, int y, Image* image, int tx, int ty, int tw, int th, float angle )
{
	Prerender();
/*
	// Ensure for gles:width/height variables
	image->serverReference( mInstance );
	int width = image->meta( "gles:width" );
	int height = image->meta( "gles:height" );
*/
	if ( tw == -1 ) {
		tw = image->width();
	}
	if ( th == -1 ) {
		th = image->height();
	}

	image->serverReference( mInstance );
	const float rx = 1.0f / image->meta( "gles:width", (float)image->width() );
	const float ry = 1.0f / image->meta( "gles:height", (float)image->height() );

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

	glBindBuffer( GL_ARRAY_BUFFER, mVBO );
	glBufferSubData( GL_ARRAY_BUFFER, 6 * sizeof(Vertex2D), 6 * sizeof(Vertex2D), vertices );

	Matrix m;
	if ( angle != 0.0f ) {
		m.Translate( x + image->width() / 2, y + image->height() / 2, 0.0f );
		m.RotateZ( -angle );
		m.Translate( -x - image->width() / 2, -y - image->height() / 2, 0.0f );
	}

	Render( image, GL_TRIANGLES, 6, 6, m );
}


void OpenGLES20Renderer2D::Draw( int x, int y, int w, int h, Image* image, int tx, int ty, int tw, int th, float angle )
{
	Prerender();
/*	// TODO ?
	if ( tx == 0 && ty == 0 && tw == -1 && th == -1 ) {
		Matrix m;
		m.Translate( x, y, 0.0f );
		if ( angle != 0.0f ) {
			m.Translate( x + w / 2, y + h / 2, 0.0f );
			m.RotateZ( -angle );
			m.Translate( -x - w / 2, -y - h / 2, 0.0f );
		}
		m.Scale( w, h, 1.0f );
		glBindBuffer( GL_ARRAY_BUFFER, mVBO );
		Render( image, 0, 6, m );
	}
*/
	if ( tw == -1 ) {
		tw = image->width();
	}
	if ( th == -1 ) {
		th = image->height();
	}

	image->serverReference( mInstance );
	const float rx = 1.0f / image->meta( "gles:width", (float)image->width() );
	const float ry = 1.0f / image->meta( "gles:height", (float)image->height() );

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

	glBindBuffer( GL_ARRAY_BUFFER, mVBO );
	glBufferSubData( GL_ARRAY_BUFFER, 6 * sizeof(Vertex2D), 6 * sizeof(Vertex2D), vertices );

	Matrix m;
	if ( angle != 0.0f ) {
		m.Translate( x + w / 2, y + h / 2, 0.0f );
		m.RotateZ( -angle );
		m.Translate( -x - w / 2, -y - h / 2, 0.0f );
	}

	Render( image, GL_TRIANGLES, 6, 6, m );
}


void OpenGLES20Renderer2D::DrawText( int x, int y, Font* font, uint32_t color, const std::string& text, Renderer2D::TextFlags flags )
{
	Prerender();

	y += font->size();
	const int base_x = x;
	const char* data = text.data();
	const uint32_t len = text.length();

	font->texture()->serverReference( mInstance );
	const float rx = 1.0f / font->texture()->meta( "gles:width", (float)font->texture()->width() );
	const float ry = 1.0f / font->texture()->meta( "gles:height", (float)font->texture()->height() );
	uint32_t iBuff = 0;

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

	Matrix matrix;
	Vertex2D vertices[6*32];
	memset( vertices, 0, sizeof(vertices) );

	for ( uint32_t i = 0; i < len; i++ ) {
		int _c = data[i];
		uint8_t c = _c;

		if ( data[i] == '\n' ) {
			x = lines_ofs ? lines_ofs[++line] : base_x;
			y += font->size();
			continue;
		}

		float sx = ( (float)font->glyph(c)->x ) * rx;
		float sy = ( (float)font->glyph(c)->y ) * ry;
		float texMaxX = ( (float)font->glyph(c)->w ) * rx;
		float texMaxY = ( (float)font->glyph(c)->h ) * ry;
		float width = font->glyph(c)->w;
		float height = font->glyph(c)->h;
		float fy = (float)y - font->glyph(c)->posY;

		vertices[iBuff + 0].u = sx;
		vertices[iBuff + 0].v = sy;
		vertices[iBuff + 0].x = x;
		vertices[iBuff + 0].y = fy;
// 		vertices[iBuff + 0].z = 0.0f;

		vertices[iBuff + 1].u = sx+texMaxX;
		vertices[iBuff + 1].v = sy+texMaxY;
		vertices[iBuff + 1].x = x+width;
		vertices[iBuff + 1].y = fy+height;
// 		vertices[iBuff + 1].z = 0.0f;

		vertices[iBuff + 2].u = sx+texMaxX;
		vertices[iBuff + 2].v = sy;
		vertices[iBuff + 2].x = x+width;
		vertices[iBuff + 2].y = fy;
// 		vertices[iBuff + 2].z = 0.0f;
		
		vertices[iBuff + 3].u = sx;
		vertices[iBuff + 3].v = sy;
		vertices[iBuff + 3].x = x;
		vertices[iBuff + 3].y = fy;
// 		vertices[iBuff + 3].z = 0.0f;

		vertices[iBuff + 4].u = sx;
		vertices[iBuff + 4].v = sy+texMaxY;
		vertices[iBuff + 4].x = x;
		vertices[iBuff + 4].y = fy+height;
// 		vertices[iBuff + 4].z = 0.0f;

		vertices[iBuff + 5].u = sx+texMaxX;
		vertices[iBuff + 5].v = sy+texMaxY;
		vertices[iBuff + 5].x = x+width;
		vertices[iBuff + 5].y = fy+height;
// 		vertices[iBuff + 5].z = 0.0f;

		vertices[iBuff + 0].color = vertices[iBuff + 1].color = vertices[iBuff + 2].color = vertices[iBuff + 3].color = vertices[iBuff + 4].color = vertices[iBuff + 5].color = 0xFFFFFFFF;

		x += font->glyph(c)->advX;
		iBuff += 6;

		if ( iBuff >= 6*32 && i + 1 < len ) {
			glBindBuffer( GL_ARRAY_BUFFER, mVBO );
			glBufferSubData( GL_ARRAY_BUFFER, 6 * sizeof(Vertex2D), iBuff * sizeof(Vertex2D), vertices );
			Render( font->texture(), GL_TRIANGLES, 6, iBuff, matrix );
			iBuff = 0;
		}
	}
	glBindBuffer( GL_ARRAY_BUFFER, mVBO );
	glBufferSubData( GL_ARRAY_BUFFER, 6 * sizeof(Vertex2D), iBuff * sizeof(Vertex2D), vertices );
	Render( font->texture(), GL_TRIANGLES, 6, iBuff, matrix );

	if ( lines_ofs ) {
		delete lines_ofs;
	}
}


void OpenGLES20Renderer2D::DrawText( int x, int y, Font* font, uint32_t color, const std::wstring& text, Renderer2D::TextFlags flags )
{
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

	Prerender();

	y += font->size();
	const int base_x = x;
	font->texture()->serverReference( mInstance );
	const float rx = 1.0f / font->texture()->meta( "gles:width", (float)font->texture()->width() );
	const float ry = 1.0f / font->texture()->meta( "gles:height", (float)font->texture()->height() );
	uint32_t iBuff = 0;

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

	Matrix matrix;
	Vertex2D vertices[6*32];
	memset( vertices, 0, sizeof(vertices) );

	for ( uint32_t i = 0; i < len; i++ ) {
		wchar_t c = data[i];

		if ( data[i] == '\n' ) {
			x = lines_ofs ? lines_ofs[++line] : base_x;
			y += font->size();
			continue;
		}

		float sx = ( (float)font->glyph(c)->x ) * rx;
		float sy = ( (float)font->glyph(c)->y ) * ry;
		float texMaxX = ( (float)font->glyph(c)->w ) * rx;
		float texMaxY = ( (float)font->glyph(c)->h ) * ry;
		float width = font->glyph(c)->w;
		float height = font->glyph(c)->h;
		float fy = (float)y - font->glyph(c)->posY;

		vertices[iBuff + 0].u = sx;
		vertices[iBuff + 0].v = sy;
		vertices[iBuff + 0].x = x;
		vertices[iBuff + 0].y = fy;
// 		vertices[iBuff + 0].z = 0.0f;

		vertices[iBuff + 1].u = sx+texMaxX;
		vertices[iBuff + 1].v = sy+texMaxY;
		vertices[iBuff + 1].x = x+width;
		vertices[iBuff + 1].y = fy+height;
// 		vertices[iBuff + 1].z = 0.0f;

		vertices[iBuff + 2].u = sx+texMaxX;
		vertices[iBuff + 2].v = sy;
		vertices[iBuff + 2].x = x+width;
		vertices[iBuff + 2].y = fy;
// 		vertices[iBuff + 2].z = 0.0f;
		
		vertices[iBuff + 3].u = sx;
		vertices[iBuff + 3].v = sy;
		vertices[iBuff + 3].x = x;
		vertices[iBuff + 3].y = fy;
// 		vertices[iBuff + 3].z = 0.0f;

		vertices[iBuff + 4].u = sx;
		vertices[iBuff + 4].v = sy+texMaxY;
		vertices[iBuff + 4].x = x;
		vertices[iBuff + 4].y = fy+height;
// 		vertices[iBuff + 4].z = 0.0f;

		vertices[iBuff + 5].u = sx+texMaxX;
		vertices[iBuff + 5].v = sy+texMaxY;
		vertices[iBuff + 5].x = x+width;
		vertices[iBuff + 5].y = fy+height;
// 		vertices[iBuff + 5].z = 0.0f;

		vertices[iBuff + 0].color = vertices[iBuff + 1].color = vertices[iBuff + 2].color = vertices[iBuff + 3].color = vertices[iBuff + 4].color = vertices[iBuff + 5].color = 0xFFFFFFFF;

		x += font->glyph(c)->advX;
		iBuff += 6;

		if ( iBuff >= 6*32 && i + 1 < len ) {
			glBindBuffer( GL_ARRAY_BUFFER, mVBO );
			glBufferSubData( GL_ARRAY_BUFFER, 6 * sizeof(Vertex2D), iBuff * sizeof(Vertex2D), vertices );
			Render( font->texture(), GL_TRIANGLES, 6, iBuff, matrix );
			iBuff = 0;
		}
	}
	glBindBuffer( GL_ARRAY_BUFFER, mVBO );
	glBufferSubData( GL_ARRAY_BUFFER, 6 * sizeof(Vertex2D), iBuff * sizeof(Vertex2D), vertices );
	Render( font->texture(), GL_TRIANGLES, 6, iBuff, matrix );

	if ( lines_ofs ) {
		delete lines_ofs;
	}
}


void OpenGLES20Renderer2D::DrawLine( int x0, int y0, uint32_t color0, int x1, int y1, uint32_t color1 )
{
	Matrix matrix;
	Vertex2D vertices[6*2];
	memset( vertices, 0, sizeof(vertices) );

	vertices[0].x = x0;
	vertices[0].y = y0;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;
	vertices[0].color = color0;

	vertices[1].x = x1;
	vertices[1].y = y1;
	vertices[1].u = 0.0f;
	vertices[1].v = 0.0f;
	vertices[1].color = color1;

	glBindBuffer( GL_ARRAY_BUFFER, mVBO );
	glBufferSubData( GL_ARRAY_BUFFER, 6 * sizeof(Vertex2D), 2 * sizeof(Vertex2D), vertices );
	Render( mTextureWhite, GL_LINES, 6, 2, matrix );
}


uintptr_t OpenGLES20Renderer2D::attributeID( const std::string& name )
{
	if ( !m2DReady ) {
		Compute();
	}
	return glGetAttribLocation( mShader, name.c_str() );
}


uintptr_t OpenGLES20Renderer2D::uniformID( const std::string& name )
{
	if ( !m2DReady ) {
		Compute();
	}
	return glGetUniformLocation( mShader, name.c_str() );
}


void OpenGLES20Renderer2D::uniformUpload( const uintptr_t id, const float f )
{
	glUseProgram( mShader );
	glUniform1f( id, f );
}


void OpenGLES20Renderer2D::uniformUpload( const uintptr_t id, const Vector2f& v )
{
	glUseProgram( mShader );
	glUniform2f( id, v.x, v.y );
}


void OpenGLES20Renderer2D::uniformUpload( const uintptr_t id, const Vector3f& v )
{
	glUseProgram( mShader );
	glUniform3f( id, v.x, v.y, v.z );
}


void OpenGLES20Renderer2D::uniformUpload( const uintptr_t id, const Vector4f& v )
{
	glUseProgram( mShader );
	glUniform4f( id, v.x, v.y, v.z, v.w );
}


void OpenGLES20Renderer2D::uniformUpload( const uintptr_t id, const Matrix& v )
{
	glUseProgram( mShader );
	glUniformMatrix4fv( id, 1, GL_FALSE, v.constData() );
}
