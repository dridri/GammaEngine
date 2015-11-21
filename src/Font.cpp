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

#include <algorithm>

#include "Image.h"
#include "Font.h"
#include "FontLoaderTtf.h"
#include "Instance.h"
#include "File.h"
#include "Debug.h"

using namespace GE;

std::vector< FontLoader* > Font::mFontLoaders = std::vector< FontLoader* >();
static bool FontLoaderFirstCall = true;


Font::Font()
	: mAllocInstance( nullptr )
	, mSize( 0 )
	, mTexture( nullptr )
	, mData( nullptr )
	, mFace( nullptr )
// 	, mGlyphs { { 0, 0, 0, 0, 0, 0 } }
	, mModInstance( nullptr )
{
	for ( uint16_t n = 0; n < 256; n++ ) {
		Glyph g = {
			.c = n,
			.x = 0,
			.y = 0,
			.w = 0,
			.h = 0,
			.advX = 0,
			.posY = 0
		};
		mGlyphs.insert( std::make_pair( n, g ) );
	}
}


Font::Font( File* file, int size, const std::string& extension, Instance* instance )
	: Font()
{
	if ( !instance ) {
		instance = Instance::baseInstance();
	}

	Load( file, size, extension, instance );
	setSize( size );
}


Font::Font( const std::string filename, int size, Instance* instance )
	: Font()
{
	if ( !instance ) {
		instance = Instance::baseInstance();
	}

	File* file = new File( filename, File::READ );
	std::string extension = filename.substr( filename.rfind( "." ) + 1 );

	Load( file, size, extension, instance );
	setSize( size );

	delete file;
}


Font::~Font()
{
}


void Font::Load( File* file, int size, const std::string& extension, Instance* instance )
{
	FontLoader* loader = nullptr;

	if ( FontLoaderFirstCall ) {
		AddFontLoader( new FontLoaderTtf() );
		FontLoaderFirstCall = false;
	}

	char first_line[32];
	file->Rewind();
	file->Read( first_line, sizeof(first_line) );
	file->Rewind();

	for ( size_t i = 0; i < mFontLoaders.size(); i++ ) {
		std::vector< std::string > patterns = mFontLoaders.at(i)->contentPatterns();
		for ( size_t j = 0; j < patterns.size(); j++ ) {
			std::string test_case = patterns[j];
			for ( size_t k = 0; k < sizeof(first_line)-test_case.length(); k++ ) {
				if ( !memcmp( first_line + k, test_case.c_str(), test_case.length() ) ) {
					loader = mFontLoaders.at(i);
					break;
				}
			}
		}
	}

	if ( !loader && extension.length() > 0 ) {
		for ( size_t i = 0; i < mFontLoaders.size(); i++ ) {
			std::vector< std::string > extensions = mFontLoaders.at(i)->extensions();
			for ( size_t j = 0; j < extensions.size(); j++ ) {
				std::string test_case = extensions[j];
				std::transform( test_case.begin(), test_case.end(), test_case.begin(), ::tolower );
				if ( extension.find( test_case ) == 0 ) {
					loader = mFontLoaders.at(i);
					break;
				}
			}
		}
	}

	if ( loader ) {
		FontLoader* modInstance = loader;
		loader = loader->NewInstance();
		loader->Load( instance, file, size );
		*this = static_cast< Font >( *loader );
		delete loader;
		mModInstance = modInstance;
		mAllocInstance = instance;
	}
}


uint32_t Font::size() const
{
	return mSize;
}


void* Font::face() const
{
	return mFace;
}


std::map< wchar_t, Font::Glyph >& Font::glyphs()
{
	return mGlyphs;
}


Font::Glyph* Font::glyph( wchar_t c ) const
{
	if ( mGlyphs.count( c ) > 0 ) {
		return (Font::Glyph*)&mGlyphs.at( c );
	}
	return nullptr;
}


Image* Font::texture() const
{
	return mTexture;
}


void Font::setSize( uint32_t size )
{
	fDebug( size, mModInstance );

	if ( size <= 0 or !mModInstance ) {
		return;
	}

	mSize = size;
	mModInstance->resize( this, size );
	mModInstance->RenderGlyphs( this );
}


void Font::RenderGlyphs()
{
	fDebug0();

	if ( !mModInstance ) {
		return;
	}

	mModInstance->RenderGlyphs( this );
}


void Font::measureString( const std::string& str, int* width, int* height )
{
	int i = 0;
	int mx = 0;
	int x = 0;
	int y = 0;

	for ( i = 0; str[i]; i++ ) {
		if ( str[i] == '\n' ) {
			if ( x > mx ) {
				mx = x;
			}
			x = 0;
			y += mSize;
			continue;
		}
		x += mGlyphs[ (uint8_t)str[i] ].advX;
	}
	if ( x > mx ) {
		mx = x;
	}
	if ( mx == 0 ) {
		mx = x;
	}

	*width = mx;
	*height = y + mSize + ( mSize * 0.4 );
}


void Font::measureString( const std::wstring& str, int* width, int* height )
{
	int i = 0;
	int mx = 0;
	int x = 0;
	int y = 0;

	for ( i = 0; str[i]; i++ ) {
		if ( str[i] == '\n' ) {
			if ( x > mx ) {
				mx = x;
			}
			x = 0;
			y += mSize;
			continue;
		}
		x += mModInstance->glyphWidth( this, str[i] );
	}
	if ( x > mx ) {
		mx = x;
	}
	if ( mx == 0 ) {
		mx = x;
	}

	*width = mx;
	*height = y + mSize + ( mSize * 0.4 );
}


void Font::Release()
{
	if ( mTexture ) {
		mTexture->Release();
	}
	if ( mData ) {
		mAllocInstance->Free( mData );
		mData = nullptr;
	}
}


Image* Font::reallocTexture( int width, int height )
{
	if ( mTexture ) {
		delete mTexture;
	}
	mTexture = new Image( width, height, 0x00000000, Instance::baseInstance() );
	return mTexture;
}


FontLoader* Font::AddFontLoader( FontLoader* loader )
{
	mFontLoaders.insert( mFontLoaders.begin(), loader );
	return loader;
}
