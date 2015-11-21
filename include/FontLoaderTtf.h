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

#ifndef FONTLOADERTTF_H
#define FONTLOADERTTF_H

#include "Font.h"

#include <sys/types.h>
//#include <freetype2/ft2build.h>
//#include <freetype2/freetype.h>
//#include <freetype2/ftglyph.h>
#include <ft2build.h>
#ifdef GE_ANDROID
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#else
#include <freetype.h>
#include <ftglyph.h>
#endif

namespace GE {

class FontLoaderTtf : public FontLoader
{
public:
	FontLoaderTtf();
	virtual std::vector< std::string > contentPatterns();
	virtual std::vector< std::string > extensions();
	virtual FontLoader* NewInstance();
	virtual void Load( Instance* instance, File* file, uint32_t size );

	virtual void resize( Font* font, int size );
	virtual void RenderGlyphs( Font* font );
	virtual uint32_t glyphWidth( Font* font, wchar_t c );

private:
	static void fontPrintTextImpl2( FT_Bitmap* bitmap, int xofs, int yofs, uint32_t* framebuffer, int width, int height );
	static FontLoaderTtf* mBaseInstance;
	static FT_Library ft_library;
};

} // namespace GE

#endif // FONTLOADERTTF_H
