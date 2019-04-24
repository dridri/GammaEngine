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

#ifndef IMAGELOADERTGA_H
#define IMAGELOADERTGA_H

#include "Image.h"
#include "File.h"
#include <sys/types.h>

namespace GE {

class ImageLoaderTga : public ImageLoader
{
public:
	ImageLoaderTga();
	virtual std::vector< std::string > contentPatterns();
	virtual std::vector< std::string > extensions();
	virtual ImageLoader* NewInstance();
	virtual void Load( Instance* instance, File* file, uint32_t pref_w, uint32_t pref_h );

private:
	int32_t mBPP;
	static ImageLoaderTga* mBaseInstance;
	Instance* mInstance;
	bool LoadTGAUncompressed_RGB( File* fTGA, uint8_t* header );
	bool LoadTGAUncompressed_Palette( File* fTGA, uint8_t* header );
	bool LoadTGAUncompressed_Gray( File* fTGA, uint8_t* header );
	bool LoadTGACompressed_RGB( File* fTGA, uint8_t* header );
	bool LoadTGACompressed_Palette( File* fTGA, uint8_t* header );
	bool LoadTGACompressed_Gray( File* fTGA, uint8_t* header );
};

} // namespace GE

#endif // IMAGELOADERTGA_H
