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

#include "Image.h"
#include "ImageLoaderPng.h"
#include "ImageLoaderJpg.h"
#include "ImageLoaderTga.h"
#include "Instance.h"
#include "File.h"
#include "Debug.h"

#include <algorithm>


namespace GE {

std::vector< ImageLoader* > Image::mImageLoaders = std::vector< ImageLoader* >();
static bool ImageLoaderFirstCall = true;
/*
#if ( defined( GE_IOS ) || defined( GE_ANDROID ) )
static uint32_t geGetNextPower2( uint32_t width )
{
	uint32_t b = width;
	int n;
	for ( n = 0; b != 0; n++ ) b >>= 1;
	b = 1 << n;
	if ( b == 2 * width ) b >>= 1;
	return b;
}
#endif
*/
Image::Image()
	: mAllocInstance( nullptr )
	, mType( ImageColor )
	, mFiltering( Linear )
	, mWidth( 0 )
	, mHeight( 0 )
	, mData( nullptr )
	, mColor( 0xFFFFFFFF )
{
}


Image::Image( File* file, const std::string& extension, Instance* instance )
	: mAllocInstance( instance ? instance : Instance::baseInstance() )
	, mType( ImageColor )
	, mFiltering( Linear )
	, mWidth( 0 )
	, mHeight( 0 )
	, mData( nullptr )
	, mColor( 0xFFFFFFFF )
{
	mServerRefs.clear();
	Load( file, extension, mAllocInstance );
}


Image::Image( const std::string& filename, Instance* instance )
	: mAllocInstance( instance ? instance : Instance::baseInstance() )
	, mType( ImageColor )
	, mFiltering( Linear )
	, mWidth( 0 )
	, mHeight( 0 )
	, mData( nullptr )
	, mColor( 0xFFFFFFFF )
{
	File* file = new File( filename, File::READ );
	std::string extension = filename.substr( filename.rfind( "." ) + 1 );

	mServerRefs.clear();
	Load( file, extension, mAllocInstance );

	delete file;
}


Image::Image( uint32_t width, uint32_t height, uint32_t backcolor, Instance* instance )
	: mAllocInstance( instance ? instance : Instance::baseInstance() )
	, mType( ImageColor )
	, mFiltering( Linear )
	, mWidth( width )
	, mHeight( height )
	, mData( nullptr )
	, mColor( 0xFFFFFFFF )
{
	mServerRefs.clear();
/*
#if ( defined( GE_IOS ) || defined( GE_ANDROID ) )
	mWidth = geGetNextPower2( mWidth );
	mHeight = geGetNextPower2( mHeight );
#endif
*/
	mData = (uint32_t*)mAllocInstance->Malloc( sizeof(uint32_t) * mWidth * mHeight );
	for ( uint32_t i = 0; i < mWidth * mHeight; i++ ) {
		mData[ i ] = backcolor;
	}
}


Image::~Image()
{
	decltype(mServerRefs)::iterator it;

	if ( mData ) {
		mAllocInstance->Free( mData );
	}
	for ( it = mServerRefs.begin(); it != mServerRefs.end(); ++it ) {
		(*it).first->UnreferenceImage( (*it).second );
	}
}


void Image::Load( File* file, const std::string& extension, Instance* instance )
{
	ImageLoader* loader = nullptr;

	if ( ImageLoaderFirstCall ) {
#ifdef GE_PNG
		AddImageLoader( new ImageLoaderPng() );
#endif
#ifdef GE_JPEG
		AddImageLoader( new ImageLoaderJpg() );
#endif
#ifdef GE_TGA
		AddImageLoader( new ImageLoaderTga() );
#endif
		ImageLoaderFirstCall = false;
	}

// 	std::string first_line = file->ReadLine();
	char first_line[32];
	file->Rewind();
	file->Read( first_line, sizeof(first_line) );
// 	std::transform( first_line.begin(), first_line.end(), first_line.begin(), ::tolower );
	file->Rewind();

	for ( size_t i = 0; i < mImageLoaders.size(); i++ ) {
		std::vector< std::string > patterns = mImageLoaders.at(i)->contentPatterns();
		for ( size_t j = 0; j < patterns.size(); j++ ) {
			std::string test_case = patterns[j];
// 			std::transform( test_case.begin(), test_case.end(), test_case.begin(), ::tolower );
// 			if ( first_line.find( test_case ) ) {
			for ( size_t k = 0; k < sizeof(first_line)-test_case.length(); k++ ) {
				if ( !memcmp( first_line + k, test_case.c_str(), test_case.length() ) ) {
					loader = mImageLoaders.at(i);
					break;
				}
			}
		}
	}

	if ( !loader && extension.length() > 0 ) {
		for ( size_t i = 0; i < mImageLoaders.size(); i++ ) {
			std::vector< std::string > extensions = mImageLoaders.at(i)->extensions();
			for ( size_t j = 0; j < extensions.size(); j++ ) {
				std::string test_case = extensions[j];
				std::transform( test_case.begin(), test_case.end(), test_case.begin(), ::tolower );
				if ( extension.find( test_case ) == 0 ) {
					loader = mImageLoaders.at(i);
					break;
				}
			}
		}
	}

	if ( loader ) {
		loader = loader->NewInstance();
		loader->Load( instance, file );
// 		*this = static_cast< Image >( *loader );
// 		*this = *((Image*)loader);
		mType = loader->mType;
		mWidth = loader->mWidth;
		mHeight = loader->mHeight;
		mData = loader->mData;
// 		delete loader;
		free( loader );
// 		gDebug() << "Image loaded, size = " << mWidth << " x " << mHeight << "\n";
	} else {
		gDebug() << "WARNING : no ImageLoader found for extension " << extension;
	}
}


Image::Type Image::type() const
{
	return mType;
}


Image::Filtering Image::filtering() const
{
	return mFiltering;
}


uint32_t Image::width() const
{
	return mWidth;
}


uint32_t Image::height() const
{
	return mHeight;
}


uint32_t* Image::data() const
{
	return mData;
}


uint32_t Image::color() const
{
	return mColor;
}


void Image::setType( const Image::Type& type )
{
	mType = type;
}


void Image::setFiltering( const Filtering& f )
{
	mFiltering = f;
}


void Image::setColor( uint32_t c )
{
	mColor = c;
}


uint64_t Image::serverReference( Instance* instance )
{
	if ( mServerRefs.find( instance ) != mServerRefs.end() ) {
		return mServerRefs[ instance ];
	}
	uint64_t ref = instance->ReferenceImage( this );
	mServerRefs.insert( std::make_pair( instance, ref ) );
	return ref;
}


void Image::Resize( uint32_t width, uint32_t height )
{
	uint32_t* data = ( uint32_t* )mAllocInstance->Malloc( sizeof(uint32_t) * width * height );

	for ( uint32_t j = 0; j < height; j++ ) {
		for ( uint32_t i = 0; i < width; i++ ) {
			// TODO / TBD : Linear lookup ?
// 			if ( j < 1 or i < 1 or j >= height - 1 or i >= width - 1 ) {
				data[ j * width + i ] = mData[ ( j * mHeight / height * mWidth ) + ( i * mWidth / width ) ];
// 			} else {
// 				uint32_t px_n1_p0 = mData[ ( ( j * mHeight / height - 1 ) * mWidth ) + ( i * mWidth / width ) + 0 ];
// 				uint32_t px_p0_n1 = mData[ ( ( j * mHeight / height + 0 ) * mWidth ) + ( i * mWidth / width ) - 1 ];
// 				uint32_t px_p1_p0 = mData[ ( ( j * mHeight / height + 1 ) * mWidth ) + ( i * mWidth / width ) + 0 ];
// 				uint32_t px_p0_p1 = mData[ ( ( j * mHeight / height + 0 ) * mWidth ) + ( i * mWidth / width ) + 1 ];
// 				uint32_t px_p0_p0 = mData[ ( j * mHeight / height * mWidth ) + ( i * mWidth / width ) ];
// 				uint64_t ret_0 = ( px_n1_p0 + px_p0_n1 ) / 2;
// 				uint64_t ret_1 = ( px_p1_p0 + px_p0_p1 ) / 2;
// 				uint64_t ret_2 = ( ret_0 + ret_1 ) / 2;
// 				data[ j * width + i ] = (uint32_t)( ( px_p0_p0 + ret_2 ) / 2 );
// 			}
		}
	}

	mAllocInstance->Free( mData );
	mData = data;
	mWidth = width;
	mHeight = height;
}


void Image::Release()
{
	if ( mServerRefs.size() == 0 ) {
		serverReference( Instance::baseInstance() );
	}
	mAllocInstance->Free( mData );
	mData = nullptr;
}


void Image::UpdateData( Instance* instance )
{
	if ( instance == nullptr ) {
		instance = Instance::baseInstance();
	}

	if ( mServerRefs.find( instance ) != mServerRefs.end() ) {
		instance->UpdateImageData( this, mServerRefs[ instance ] );
	}
}


ImageLoader* Image::AddImageLoader( ImageLoader* loader )
{
	mImageLoaders.insert( mImageLoaders.begin(), loader );
	return loader;
}

} // namespace GE
