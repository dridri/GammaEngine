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

#include <cmath>
#include "FontLoaderTtf.h"
#include "File.h"
#include "Image.h"
#include "Instance.h"
#include "Debug.h"

namespace GE {

FT_Library FontLoaderTtf::ft_library;


FontLoaderTtf::FontLoaderTtf()
	: FontLoader()
{
	FT_Init_FreeType( &ft_library );
}


std::vector< std::string > FontLoaderTtf::contentPatterns()
{
	std::vector< std::string > ret;
	return ret;
}


std::vector< std::string > FontLoaderTtf::extensions()
{
	std::vector< std::string > ret;
	ret.emplace_back( "ttf" );
	ret.emplace_back( "otf" );
	return ret;
}


FontLoader* FontLoaderTtf::NewInstance()
{
	return new FontLoaderTtf();
}


void FontLoaderTtf::Load( Instance* instance, File* file, uint32_t size )
{
	fDebug( instance, file, size );

	uint64_t file_size = file->Seek( 0, File::END );
	mData = (uint8_t*)instance->Malloc( file_size );
	file->Rewind();
	file->Read( mData, file_size );

	FT_Face face;
	int error = FT_New_Memory_Face( ft_library, mData, file_size, 0, &face );
	if(error){
		instance->Free( mData );
		return;
	}

	mFace = face;
	mTexture = nullptr;

	FT_Select_Charmap( ((FT_Face)mFace), FT_ENCODING_UNICODE );
}


void FontLoaderTtf::resize( Font* font, int size )
{
	fDebug( font, size );

	FT_Set_Pixel_Sizes( (FT_Face)font->face(), 0, size );
}


void FontLoaderTtf::RenderGlyphs( Font* font )
{
	fDebug( font );

	FT_Set_Pixel_Sizes( (FT_Face)font->face(), 0, font->size() );

	FT_Face face = (FT_Face)font->face();
	FT_GlyphSlot slot = face->glyph;
	std::map< wchar_t, Font::Glyph >& glyphs = font->glyphs();
	uint16_t n;
	int x = 0;
	int y = 0;
	bool first_null_char = true;

	int total_width = 0;
	int advY = 0;
	int advX = 0;
	int count = 0;

	for ( std::map< wchar_t, Font::Glyph >::const_iterator it = glyphs.begin(); it != glyphs.end(); it++, count++ ) {
		n = (*it).first;
		FT_UInt glyph_index = FT_Get_Char_Index( face, n );
		int error = FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT );
		if ( error ) continue;
		error = FT_Render_Glyph( slot, FT_RENDER_MODE_NORMAL );
		if ( error ) continue;
		if ( !glyph_index && !first_null_char ) {
			continue;
		}
		advX = std::max( advX, std::max( (int)slot->advance.x >> 6, std::max( (int)slot->bitmap.width, (int)font->size() ) ) );
		advY = std::max( advY, (int)slot->bitmap.rows );
		total_width += advX;
		first_null_char = false;
	}

//	int side = std::max( advX * 16, advY * 16 );
//	Image* texture = font->reallocTexture( side, side );
	int width = advX * 2 + std::sqrt( (double)( advX * advY * count ) );
	int height = advY * 2 + std::sqrt( (double)( advX * advY * count ) );
	Image* texture = font->reallocTexture( width, height );
// 	Image* texture = font->reallocTexture( advX * 16, advY * 16 );

	first_null_char = true;
	y = advY;

	for ( std::map< wchar_t, Font::Glyph >::const_iterator it = glyphs.begin(); it != glyphs.end(); it++ ) {
		n = (*it).first;
		FT_UInt glyph_index = FT_Get_Char_Index( face, n );
		int error = FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT );
		if ( error ) {
			memcpy( font->glyph(n), font->glyph(0), sizeof( Font::Glyph ) );
			font->glyph(n)->c = n;
			continue;
		}
		error = FT_Render_Glyph( slot, FT_RENDER_MODE_NORMAL );
		if ( error ) {
			memcpy( font->glyph(n), font->glyph(0), sizeof( Font::Glyph ) );
			font->glyph(n)->c = n;
			continue;
		}

		if ( (uint32_t)( x + advX ) > texture->width() ) {
			x = 0;
			y += advY;
		}

		font->glyph(n)->x = std::max( x, 0 );
		font->glyph(n)->y = y - slot->bitmap_top;
		font->glyph(n)->w = slot->bitmap.width + slot->bitmap_left + ( font->outlineColor() ? 4 : 0 );
		font->glyph(n)->h = slot->bitmap.rows;
		font->glyph(n)->advX = (slot->advance.x >> 6);
		font->glyph(n)->posY = slot->bitmap_top;

		if ( !glyph_index && !first_null_char ) {
			memcpy( font->glyph(n), font->glyph(0), sizeof( Font::Glyph ) );
			font->glyph(n)->c = n;
			continue;
		}

		fontPrintTextImpl2( &slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top, texture->data(), texture->width(), texture->height(), 0xFFFFFFFF, font->outlineColor() );

		x += advX;
		first_null_char = false;
	}
}


void FontLoaderTtf::fontPrintTextImpl2( FT_Bitmap* bitmap, int xofs, int yofs, uint32_t* framebuffer, int width, int height, uint32_t color, uint32_t outline, uint32_t buf_bpp, bool reverse )
{
	int x, y;
	int bpp_step = buf_bpp / 8;

	uint16_t color16 = ( ( color & 0xF80000 ) >> 19 ) | ( ( color & 0xFC00 ) >> 5 ) | ( ( color & 0xF8 ) << 8 );

	if ( reverse ) {
		framebuffer += width * height * bpp_step / 4;
		bpp_step = -bpp_step;
	}

	uint8_t* line = bitmap->buffer;
	uint8_t* fbLine = ((uint8_t*)framebuffer) + ( xofs + yofs * width ) * bpp_step;
	for ( y = 0; y < (int)bitmap->rows; y++ ) {
		uint8_t* column = line;
		uint8_t* fbColumn = fbLine;
		for ( x = 0; x < (int)bitmap->width; x++ ) {
			if ( x + xofs < width && x + xofs >= 0 && y + yofs < height && y + yofs >= 0 ) {
				uint8_t val = *column;
				if ( val >= 0x7F ) {
					switch ( buf_bpp ) {
						case 16 :
							*((uint16_t*)fbColumn) = color16;
							break;
						default :
							*((uint32_t*)fbColumn) = ( val << 24 ) | ( color & 0x00FFFFFF );
							break;
					}
				}
			}
			column++;
			fbColumn += bpp_step;
		}
		line += bitmap->pitch;
		fbLine += width * bpp_step;
	}

	if ( outline ) {
		line = bitmap->buffer;
		fbLine = ((uint8_t*)framebuffer) + ( ( xofs - 2 ) + ( yofs - 2 ) * width ) * bpp_step;
		for ( y = 0; y < (int)bitmap->rows + 2; y++ ) {
			uint8_t* column = line;
			uint8_t* fbColumn = fbLine;
			for ( x = 0; x < (int)bitmap->width + 2; x++ ) {
				switch ( buf_bpp ) {
					case 16 :
						// TODO
						break;
					default :
						uint32_t right = *((uint32_t*)(fbColumn+bpp_step));
						uint32_t left = *((uint32_t*)(fbColumn-bpp_step));
						uint32_t bottom = *((uint32_t*)(fbColumn+width*bpp_step));
						uint32_t top = *((uint32_t*)(fbColumn-width*bpp_step));
						if ( ( *((uint32_t*)fbColumn) & 0xFF000000 ) == 0 and ( right & 0xFF000000 ) != 0 ) {
							*((uint32_t*)fbColumn) = outline;
						}
						if ( ( *((uint32_t*)fbColumn) & 0xFF000000 ) == 0 and ( left & 0xFF000000 ) != 0 and left != outline ) {
							*((uint32_t*)fbColumn) = outline;
						}
						if ( ( *((uint32_t*)fbColumn) & 0xFF000000 ) == 0 and ( bottom & 0xFF000000 ) != 0 ) {
							*((uint32_t*)fbColumn) = outline;
						}
						if ( ( *((uint32_t*)fbColumn) & 0xFF000000 ) == 0 and ( top & 0xFF000000 ) != 0 and top != outline ) {
							*((uint32_t*)fbColumn) = outline;
						}
						break;
				}
				column++;
				fbColumn += bpp_step;
			}
			line += bitmap->pitch;
			fbLine += width * bpp_step;
		}
	}
}


void FontLoaderTtf::RenderCharacter( Font* font, const char c, uint32_t color, uint32_t* buffer, uint32_t xofs, uint32_t yofs, uint32_t buf_width, uint32_t buf_height, uint32_t buf_bpp, bool reverse )
{
	FT_Face face = (FT_Face)font->face();
	FT_GlyphSlot slot = face->glyph;
	wchar_t n = c;

	FT_UInt glyph_index = FT_Get_Char_Index( face, n );
	int error = FT_Load_Glyph( face, glyph_index, FT_LOAD_DEFAULT );
	if ( error ) {
		return;
	}
	error = FT_Render_Glyph( slot, FT_RENDER_MODE_NORMAL );
	if ( error ) {
		return;
	}

	fontPrintTextImpl2( &slot->bitmap, xofs + slot->bitmap_left, yofs - slot->bitmap_top, buffer, buf_width, buf_height, color, font->outlineColor(), buf_bpp, reverse );
}


uint32_t FontLoaderTtf::glyphWidth( Font* font, wchar_t c )
{
	if ( font->glyphs().count( c ) <= 0 ) {
		Font::Glyph g = {
			.c = c,
			.x = 0,
			.y = 0,
			.w = 0,
			.h = 0,
			.advX = 0,
			.posY = 0
		};
		font->glyphs().insert( std::make_pair( c, g ) );
		font->RenderGlyphs();
	}
	return font->glyph( c )->advX;
/*
	FT_Set_Pixel_Sizes( (FT_Face)font->face(), 0, font->size() );

	FT_UInt glyph_index = FT_Get_Char_Index( (FT_Face)font->face(), c );
	int error = FT_Load_Glyph( (FT_Face)font->face(), glyph_index, FT_LOAD_DEFAULT );
	if ( error ) { gDebug() << "err1 " << error << " (" << font->size() << ")\n"; return 0; }
// 	error = FT_Render_Glyph( (FT_Face)font->face()->glyph, FT_RENDER_MODE_NORMAL );
// 	if ( error ) { gDebug() << "err2 " << error << "\n"; return 0; }
	gDebug() << "ret : " << ( ((FT_Face)font->face())->glyph->advance.x >> 6 ) << "\n";
	return font->size();
	return ( ((FT_Face)font->face())->glyph->advance.x >> 6 );
*/
}



} // namespace GE
