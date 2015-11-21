/*
 * The GammaEngine Library 2.0 is a multiplatform Vulkan-based game engine
 * Copyright (C) 2015  Adrien Aubry <email>
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

#ifndef GE_FONT_H
#define GE_FONT_H

#include <string>
#include <vector>
#include <map>

namespace GE {

class Instance;
class FontLoader;
class File;
class Image;

class Font
{
public:
	typedef struct Glyph {
		wchar_t c;
		int x, y, w, h, advX, posY;
	} Glyph;

	Font();
	Font( File* file, int size = 16, const std::string& extension = "", Instance* instance = nullptr );
	Font( const std::string filename, int size = 16, Instance* instance = nullptr );
	~Font();

	uint32_t size() const;
	void* face() const;
	std::map< wchar_t, Glyph >& glyphs();
	Glyph* glyph( wchar_t c ) const;
	Image* texture() const;
	void setSize( uint32_t size );
	void RenderGlyphs();

	void measureString( const std::string& str, int* width, int* height );
	void measureString( const std::wstring& str, int* width, int* height );
	void Release();

	Image* reallocTexture( int width, int height );
	static FontLoader* AddFontLoader( FontLoader* loader );

protected:
	void Load( File* file, int size, const std::string& extension, Instance* instance );

	Instance* mAllocInstance;
	int mSize;
	Image* mTexture;
	uint8_t* mData;
	void* mFace;
// 	Glyph mGlyphs[256];
	std::map< wchar_t, Glyph > mGlyphs;
	FontLoader* mModInstance;

	static std::vector< FontLoader* > mFontLoaders;
};


class FontLoader : public Font
{
public:
	FontLoader() : Font() { ; }
	virtual ~FontLoader() { ; }
	virtual std::vector< std::string > contentPatterns() = 0;
	virtual std::vector< std::string > extensions() = 0;
	virtual FontLoader* NewInstance() = 0;
	virtual void Load( Instance* instance, File* file, uint32_t size ) = 0;

	virtual void resize( Font* font, int size ) = 0;
	virtual void RenderGlyphs( Font* font ) = 0;
	virtual uint32_t glyphWidth( Font* font, wchar_t c ) = 0;
};

}

#endif // GE_FONT_H
