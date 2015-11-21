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

#include "ImageLoaderPng.h"
#include "File.h"
#include "Instance.h"
#include "Debug.h"

namespace GE {


std::map< uintptr_t, std::pair< uintptr_t, uintptr_t > > ImageLoaderPng::mPngAllocs = std::map< uintptr_t, std::pair< uintptr_t, uintptr_t > >();


ImageLoaderPng::ImageLoaderPng()
	: ImageLoader()
{
}


std::vector< std::string > ImageLoaderPng::contentPatterns()
{
	std::vector< std::string > ret;
	ret.emplace_back( "PNG" );
	return ret;
}


std::vector< std::string > ImageLoaderPng::extensions()
{
	std::vector< std::string > ret;
	ret.emplace_back( "png" );
	return ret;
}


ImageLoader* ImageLoaderPng::NewInstance()
{
	return new ImageLoaderPng();
}


void ImageLoaderPng::png_user_error_fn( png_structp png_ptr, png_const_charp error_msg )
{
	gDebug() << "PNG Error: " << error_msg << "\n";
}


void ImageLoaderPng::png_user_warning_fn( png_structp png_ptr, png_const_charp warning_msg )
{
	gDebug() << "PNG Warning: " << warning_msg << "\n";
}


void ImageLoaderPng::png_read_from_File( png_structp png_ptr, png_bytep data, png_size_t length )
{
	File* file = ( File* )png_get_io_ptr( png_ptr );
	file->Read( data, length );
}


// There is a memory leak in libpng, using homemade malloc crashes because libpng makes buffer overflow above address
// So we store real malloc'ed address in a structure list
png_voidp ImageLoaderPng::ge_png_malloc( png_structp png_ptr, png_size_t size )
{
	Instance* instance = (Instance*)png_ptr;
//	return instance->Malloc(size);

	uintptr_t* buf = ( uintptr_t* )instance->Malloc( size * 4 );
	uintptr_t ptr = (uintptr_t)buf;

	mPngAllocs.insert( std::pair< uintptr_t, std::pair< uintptr_t, uintptr_t > >( ptr, std::pair< uintptr_t, uintptr_t >( buf[-2], buf[-1] ) ) );

	return (void*)buf;

}


void ImageLoaderPng::ge_png_free( png_structp png_ptr, png_voidp data )
{
	Instance* instance = (Instance*)png_ptr;

//	instance->Free(data);
//	return;

	decltype(mPngAllocs)::iterator it = mPngAllocs.find( (uintptr_t)data );

	if ( it != mPngAllocs.end() ) {
		((uintptr_t*)data)[-2] = (*it).second.first;
		((uintptr_t*)data)[-1] = (*it).second.second;
		instance->Free( data );
		mPngAllocs.erase( it );
	} else {
		gDebug() << "err\n";
	}

}


void ImageLoaderPng::Load( Instance* instance, File* file, uint32_t pref_w, uint32_t pref_h )
{
	fDebug( instance, file, pref_w, pref_h );
	if ( !instance ) {
		instance = Instance::baseInstance();
	}

	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height, x, y;
	int bit_depth, color_type, interlace_type;
	uint32_t* line;

	uint8_t magic[8] = { 0x0 };
	file->Read( magic, 8 );
	file->Rewind();
#ifdef png_check_sig
	if ( !png_check_sig( magic, 8 ) ) {
		return;
	}
#endif

	gDebug() << "libpng version : " << PNG_LIBPNG_VER_STRING << "\n";

	png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, nullptr, &png_user_error_fn, &png_user_warning_fn );
	if ( png_ptr == nullptr ) {
		gDebug() << "PNG Error: png_create_read_struct returned null\n";
		return;
	}

#ifndef GE_IOS
// 	png_set_mem_fn( png_ptr, nullptr, ge_png_malloc, ge_png_free );
#endif
	png_set_error_fn( png_ptr, nullptr, nullptr, nullptr );
	png_set_read_fn( png_ptr, ( png_voidp* )file, png_read_from_File );

	if ( ( info_ptr = png_create_info_struct( png_ptr ) ) == nullptr ) {
		png_destroy_read_struct( &png_ptr, nullptr, nullptr );
		return;
	}

//	png_read_png( png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr );
	png_read_info( png_ptr, info_ptr );

	png_get_IHDR( png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, nullptr, nullptr );
	mWidth = ( pref_w > 0 ) ? pref_w : width;
	mHeight = ( pref_h > 0 ) ? pref_h : height;

	mData = ( uint32_t* ) instance->Malloc( mWidth * mHeight * sizeof( uint32_t ) );
	if ( !mData ) {
		instance->Free( mData );
		png_destroy_read_struct( &png_ptr, nullptr, nullptr );
		return;
	}

	png_set_strip_16( png_ptr );
	png_set_packing( png_ptr );
	if ( color_type == PNG_COLOR_TYPE_PALETTE ) png_set_palette_to_rgb( png_ptr );
	if ( png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS ) ) png_set_tRNS_to_alpha( png_ptr );
	png_set_filler( png_ptr, 0xff, PNG_FILLER_AFTER );

	line = ( uint32_t* ) instance->Malloc( width * sizeof( uint32_t ) );
	if ( !line ) {
		instance->Free( mData );
		png_destroy_read_struct( &png_ptr, nullptr, nullptr );
		return;
	}

	for ( y = 0; y < height; y++ ) {
		png_read_row( png_ptr, (uint8_t*)line, nullptr );
		for ( x = 0; x < width; x++ ) {
			uint32_t color = line[x];
			uint32_t x2, y2;
			for ( y2=( y*mHeight/height ); y2<( ( y+1 )*mHeight/height ); y2++ ) {
				for ( x2=( x*mWidth/width ); x2<( ( x+1 )*mWidth/width ); x2++ ) {
					mData[ x2 + y2 * mWidth ] = color;
				}
			}
// 			mData[ x + y * mWidth] = color;
		}
	}

	instance->Free( line );
	png_read_end( png_ptr, info_ptr );
	png_destroy_read_struct( &png_ptr, &info_ptr, nullptr );
}


} // namespace GE
