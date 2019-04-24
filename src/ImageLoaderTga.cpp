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

#include "ImageLoaderTga.h"
#include "File.h"
#include "Instance.h"
#include "Debug.h"

#define ABGR(a,b,g,r) (((a) << 24)|((b) << 16)|((g) << 8)|(r))
#define ARGB(a,r,g,b) ABGR((a),(b),(g),(r))
#define RGBA(r,g,b,a) ARGB((a),(r),(g),(b))
#define RGB(r,g,b) RGBA(r,g,b,255)
#define RGBf(r,g,b) RGBA((u8)((r)*255.0f),(u8)((g)*255.0f),(u8)((b)*255.0f),255)
#define RGBAf(r,g,b,a) RGBA((u8)((r)*255.0f),(u8)((g)*255.0f),(u8)((b)*255.0f),(u8)((a)*255.0f))
#define fRGBA(f) RGBAf(f[0], f[1], f[2], f[3])

namespace GE {


ImageLoaderTga::ImageLoaderTga()
	: ImageLoader()
{
}


std::vector< std::string > ImageLoaderTga::contentPatterns()
{
	std::vector< std::string > ret;
	return ret;
}


std::vector< std::string > ImageLoaderTga::extensions()
{
	std::vector< std::string > ret;
	ret.emplace_back( "tga" );
	ret.emplace_back( "TGA" );
	return ret;
}


ImageLoader* ImageLoaderTga::NewInstance()
{
	return new ImageLoaderTga();
}


void ImageLoaderTga::Load( Instance* instance, File* file, uint32_t pref_w, uint32_t pref_h )
{
	fDebug( instance, file, pref_w, pref_h );
	if ( !instance ) {
		instance = Instance::baseInstance();
	}
	mInstance = instance;

	uint8_t header[20] = { 0 };
	file->Read( header, 18 );

	mWidth = header[13] * 256 + header[12];
	mHeight = header[15] * 256 + header[14];
	mBPP = header[16] / 8;
	gDebug() << "TGA data : " << mWidth << ", " << mHeight << ", " << mBPP;

	if ( mWidth <= 0 or mHeight <= 0 ) {
		return;
	}

	bool ok = false;
	if ( header[2] == 1 ) {
		ok = LoadTGAUncompressed_Palette( file, header );
	}else
	if ( header[2] == 2 ) {
		ok = LoadTGAUncompressed_RGB( file, header );
	}else
	if ( header[2] == 3 ) {
		ok = LoadTGAUncompressed_Gray( file, header );
	}else
	if ( header[2] == 9 ) {
		ok = LoadTGACompressed_Palette( file, header );
	}else
	if ( header[2] == 10 ) {
		ok = LoadTGACompressed_RGB( file, header );
	}else
	if ( header[2] == 11 ) {
		ok = LoadTGACompressed_Gray( file, header );
	}
	if ( !ok ) {
		gDebug() << "WARNING : cannot load file !";
	}

	int size = mWidth * mHeight;
	int line_size = sizeof(uint32_t) * mWidth;
	uint8_t* buffer = (uint8_t*)instance->Malloc( line_size );
	int32_t i = 0;
	int32_t j = size - mWidth;
	for ( int32_t y = 0; y < mHeight / 2; y++ ) {
		memcpy( buffer, &mData[i], line_size );
		memcpy( &mData[i], &mData[j], line_size );
		memcpy( &mData[j], buffer, line_size );
		i += mWidth;
		j -= mWidth;
		if ( i >= ( ( size * 4 ) - line_size ) ) {
			break;
		}
	}

	instance->Free( buffer );
}


bool ImageLoaderTga::LoadTGAUncompressed_RGB( File* fTGA, uint8_t* header )
{
	fDebug( fTGA, header );

	int size = mBPP * mWidth * mHeight;
	uint8_t* buffer = (uint8_t*)mInstance->Malloc(mBPP * mWidth * mHeight);
	fTGA->Read( buffer, size );

	mData = ( uint32_t* )mInstance->Malloc( mWidth * mHeight * sizeof( uint32_t ) );

	if ( mBPP >= 3 ) {
		// Byte Swapping Optimized By Steve Thomas
		int cswap = 0;
		for(cswap = 0; cswap < size; cswap += mBPP){
			buffer[cswap] ^= buffer[cswap+2];
			buffer[cswap+2] ^= buffer[cswap];
			buffer[cswap] ^= buffer[cswap+2];
		}
	}

	if ( mBPP == 1 ) {
		memcpy(mData, buffer, size);
		uint8_t* buf2 = (uint8_t*)mData;
		int s, d;
		for(s=0, d=0; s<size; s++, d+=3){
			buffer[d] = buf2[s];
			buffer[d+1] = buf2[s];
			buffer[d+2] = buf2[s];
		}
		mBPP = 3;
		size = mBPP * mWidth * mHeight;
		memset(mData, 0, sizeof(uint32_t)*mWidth*mHeight);
	} else if ( mBPP == 2 ) {
		memcpy(mData, buffer, size);
		uint8_t* buf2 = (uint8_t*)mData;
		int s, d;
		for(s=0, d=0; s<size; s+=2, d+=3){
			uint16_t color = buf2[s] + (buf2[s+1]<<8);
			buffer[d] = (uint8_t)(((color & 0x7C00) >> 10)<<3);
			buffer[d+1] = (uint8_t)(((color & 0x03E0) >> 5)<<3);
			buffer[d+2] = (uint8_t)((color & 0x001F)<<3);
		}
		mBPP = 3;
		size = mBPP * mWidth * mHeight;
		memset(mData, 0, sizeof(uint32_t)*mWidth*mHeight);
	}

	int i, j;
	int src = 0;
	int dst = 0;
	for(j=0; j<mHeight; j++, dst+=(mWidth-mWidth), src+=0){
		for(i=0; i<mWidth && src<size; i++,dst++,src+=mBPP){
			if(mBPP == 4){
				mData[dst] = RGBA(buffer[src], buffer[src+1], buffer[src+2], buffer[src+3]);
			} else if(mBPP == 3){
				mData[dst] = RGBA(buffer[src], buffer[src+1], buffer[src+2], 255);
			}
		}
	}
	
	mInstance->Free(buffer);
	return true;
}


bool ImageLoaderTga::LoadTGAUncompressed_Palette( File* fTGA, uint8_t* header )
{
	fDebug( fTGA, header );
}


bool ImageLoaderTga::LoadTGAUncompressed_Gray( File* fTGA, uint8_t* header )
{
	fDebug( fTGA, header );
}


bool ImageLoaderTga::LoadTGACompressed_RGB( File* fTGA, uint8_t* header )
{
	fDebug( fTGA, header );
}


bool ImageLoaderTga::LoadTGACompressed_Palette( File* fTGA, uint8_t* header )
{
	fDebug( fTGA, header );
}


bool ImageLoaderTga::LoadTGACompressed_Gray( File* fTGA, uint8_t* header )
{
	fDebug( fTGA, header );
}


} // namespace GE
