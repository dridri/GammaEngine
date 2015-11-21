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

#ifndef IMAGELOADERJPG_H
#define IMAGELOADERJPG_H

#include "Image.h"

#include <sys/types.h>
#include <jpeglib.h>
#include <jerror.h>

namespace GE {

class ImageLoaderJpg : public ImageLoader
{
public:
	ImageLoaderJpg();
	virtual std::vector< std::string > contentPatterns();
	virtual std::vector< std::string > extensions();
	virtual ImageLoader* NewInstance();
	virtual void Load( Instance* instance, File* file, uint32_t pref_w, uint32_t pref_h );

private:
	typedef struct ge_jpeg_src {
		struct jpeg_source_mgr jsrc;
		File* file;
		uint8_t* buffer;
		int start_of_file;
	} ge_jpeg_src;

	static ImageLoaderJpg* mBaseInstance;
	static void ge_init_jpeg_src( Instance* instance, struct jpeg_decompress_struct* dinfo, File* file);
	static void init_source( j_decompress_ptr cinfo );
	static boolean file_fill_input_buffer( j_decompress_ptr cinfo );
	static void skip_input_data( j_decompress_ptr cinfo, long num_bytes );
	static void term_source( j_decompress_ptr cinfo );
};

} // namespace GE

#endif // IMAGELOADERJPG_H
