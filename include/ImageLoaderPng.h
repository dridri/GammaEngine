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

#ifndef IMAGELOADERPNG_H
#define IMAGELOADERPNG_H

#include <png.h>

#include "Image.h"

namespace GE {

class ImageLoaderPng : public ImageLoader
{
public:
	ImageLoaderPng();
	virtual std::vector< std::string > contentPatterns();
	virtual std::vector< std::string > extensions();
	virtual ImageLoader* NewInstance();
	virtual void Load( Instance* instance, File* file, uint32_t pref_w, uint32_t pref_h );

private:
	static ImageLoaderPng* mBaseInstance;
	static std::map< uintptr_t, std::pair< uintptr_t, uintptr_t > > mPngAllocs;
	static void png_user_error_fn( png_structp png_ptr, png_const_charp error_msg );
	static void png_user_warning_fn( png_structp png_ptr, png_const_charp warning_msg );
	static void png_read_from_File( png_structp png_ptr, png_bytep data, png_size_t length );
	static png_voidp ge_png_malloc( png_structp png_ptr, png_size_t size );
	static void ge_png_free( png_structp png_ptr, png_voidp data );
};

} // namespace GE

#endif // IMAGELOADERPNG_H
