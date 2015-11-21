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

#include "ImageLoaderJpg.h"
#include "File.h"
#include "Instance.h"
#include "Debug.h"

namespace GE {


ImageLoaderJpg::ImageLoaderJpg()
	: ImageLoader()
{
}


std::vector< std::string > ImageLoaderJpg::contentPatterns()
{
	std::vector< std::string > ret;
	ret.emplace_back( "JFIF" );
	ret.emplace_back( "Exif" );
	return ret;
}


std::vector< std::string > ImageLoaderJpg::extensions()
{
	std::vector< std::string > ret;
	ret.emplace_back( "jpg" );
	ret.emplace_back( "jpeg" );
	return ret;
}


ImageLoader* ImageLoaderJpg::NewInstance()
{
	return new ImageLoaderJpg();
}


void ImageLoaderJpg::Load( Instance* instance, File* file, uint32_t pref_w, uint32_t pref_h )
{
	fDebug( instance, file, pref_w, pref_h );
	if ( !instance ) {
		instance = Instance::baseInstance();
	}

	struct jpeg_decompress_struct dinfo;
	struct jpeg_error_mgr jerr;

	dinfo.err = jpeg_std_error( &jerr );
	jpeg_create_decompress( &dinfo );

	ge_init_jpeg_src( instance, &dinfo, file );

	jpeg_read_header( &dinfo, TRUE );
	mWidth = dinfo.image_width;
	mHeight = dinfo.image_height;
	jpeg_start_decompress( &dinfo );

	mData = ( uint32_t* )instance->Malloc( mWidth * mHeight * sizeof( uint32_t ) );

	if ( !mData ) {
		jpeg_destroy_decompress( &dinfo );
		return;
	}

	uint8_t* line = ( uint8_t* )instance->Malloc( mWidth * sizeof( uint8_t ) * 4 );
	if ( !line ) {
		instance->Free( mData );
		jpeg_destroy_decompress( &dinfo );
		return;
	}
	if ( dinfo.jpeg_color_space == JCS_GRAYSCALE ) {
		while ( dinfo.output_scanline < dinfo.output_height ) {
			int y = dinfo.output_scanline;
			jpeg_read_scanlines( &dinfo, &line, 1 );
			uint32_t x;
			for ( x = 0; x < mWidth; x++ ) {
				uint32_t c = line[x];
				mData[x + mWidth * y] = c | (c << 8) | (c << 16) | 0xff000000;
			}
		}
	} else {
		while ( dinfo.output_scanline < dinfo.output_height ) {
			int y = dinfo.output_scanline;
			jpeg_read_scanlines(&dinfo, &line, 1);
			uint8_t* linePointer = line;
			uint32_t x;
			for ( x = 0; x < mWidth; x++ ) {
				uint32_t c = *(linePointer++);
				c |= (*(linePointer++)) << 8;
				c |= (*(linePointer++)) << 16;
				mData[x + mWidth * y] = c | 0xff000000;
			}
		}
	}
	jpeg_finish_decompress( &dinfo );
	jpeg_destroy_decompress( &dinfo );
	instance->Free( ((ge_jpeg_src*)dinfo.src)->buffer );
	instance->Free( dinfo.src );
	instance->Free( line );
}


void ImageLoaderJpg::ge_init_jpeg_src( Instance* instance, struct jpeg_decompress_struct* dinfo, File* file )
{
	dinfo->src = (struct jpeg_source_mgr*)instance->Malloc( sizeof(ge_jpeg_src) );

	ge_jpeg_src* src = (ge_jpeg_src*)dinfo->src;
	src->file = file;
	src->buffer = (uint8_t*)instance->Malloc( 4096 );
	src->start_of_file = true;

	src->jsrc.next_input_byte = nullptr;
	src->jsrc.bytes_in_buffer = 0;
	src->jsrc.init_source = init_source;
	src->jsrc.fill_input_buffer = file_fill_input_buffer;
	src->jsrc.skip_input_data = skip_input_data;
	src->jsrc.resync_to_restart = jpeg_resync_to_restart;
	src->jsrc.term_source = term_source;
}


void ImageLoaderJpg::init_source( j_decompress_ptr cinfo )
{
}


void ImageLoaderJpg::term_source( j_decompress_ptr cinfo )
{
}


boolean ImageLoaderJpg::file_fill_input_buffer( j_decompress_ptr cinfo )
{
	ge_jpeg_src* src = (ge_jpeg_src*)cinfo->src;

	int nbytes = src->file->Read( src->buffer, 4096 );

	if ( nbytes <= 0 ) {
		if ( src->start_of_file ) { //It's an error
			return FALSE;
		}
		WARNMS(cinfo, JWRN_JPEG_EOF);
		// Insert a fake EOI marker
		src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;
		nbytes = 2;
	}

	src->jsrc.next_input_byte = src->buffer;
	src->jsrc.bytes_in_buffer = nbytes;
	src->start_of_file = FALSE;

	return TRUE;
}


void ImageLoaderJpg::skip_input_data( j_decompress_ptr cinfo, long num_bytes )
{
	struct jpeg_source_mgr *jsrc = cinfo->src;

	if ( num_bytes > 0 ) {
		while ( num_bytes > (long)jsrc->bytes_in_buffer ) {
			num_bytes -= (long)jsrc->bytes_in_buffer;
			file_fill_input_buffer(cinfo);
		}
		jsrc->next_input_byte += num_bytes;
		jsrc->bytes_in_buffer -= num_bytes;
	}
}


} // namespace GE
