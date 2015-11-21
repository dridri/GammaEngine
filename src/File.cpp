/*
 * <one line to give the library's name and an idea of what it does.>
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

#ifdef GE_ANDROID
#include "android/BaseWindow.h"
#endif

#include "Instance.h"
#include "File.h"
#include "Debug.h"
#include <errno.h>
#include <string.h>

#ifdef GE_IOS
std::fstream* _ge_iOSOpen( const char* file, std::ios_base::openmode mode );
#endif

namespace GE {


File::File( std::string filename, MODE mode )
	: mType( FILE )
	, mMode( mode )
	, mStream( nullptr )
{
	fDebug( filename, mode );

	std::ios_base::openmode std_mode = std::ios_base::binary;

	if ( mode & READ ) {
		std_mode |= std::ios_base::in;
	}
	if ( mode & WRITE ) {
		std_mode |= std::ios_base::out;
	}

	mPath = filename;
#ifdef GE_ANDROID
	mIsAsset = false;
	mStream = new std::fstream( filename, std_mode );
	if ( !mStream->is_open() ) {
		mStream = new std::fstream( std::string( BaseWindow::androidState()->activity->internalDataPath ) + "/" + filename, std_mode );
	}
	if ( !mStream->is_open() ) {
		mStream = (std::fstream*)AAssetManager_open( BaseWindow::androidState()->activity->assetManager, filename.c_str(), AASSET_MODE_STREAMING );
		mIsAsset = true;
	}
#elif GE_IOS
	mStream = new std::fstream( filename, std_mode );
	if ( !mStream->is_open() ) {
		delete mStream;
		mStream = _ge_iOSOpen( filename.c_str(), std_mode );
	}
#else
	mStream = new std::fstream( filename, std_mode );
#endif
}


File::File( File* side, std::string filename, File::MODE mode )
	: mType( FILE )
	, mMode( mode )
{
	fDebug( side, filename, mode );

	std::string path = side->mPath.substr( 0, side->mPath.rfind( "/" ) + 1 ) + filename;

	gDebug() << "Resulting path : '" << path << "'\n";

	std::ios_base::openmode std_mode = std::ios_base::binary;

	if ( mode & READ ) {
		std_mode |= std::ios_base::in;
	}
	if ( mode & WRITE ) {
		std_mode |= std::ios_base::out;
	}

	mPath = path;
#ifdef GE_ANDROID
	mIsAsset = false;
	mStream = new std::fstream( filename, std_mode );
	if ( !mStream->is_open() ) {
		mStream = new std::fstream( std::string( BaseWindow::androidState()->activity->internalDataPath ) + "/" + filename, std_mode );
	}
	if ( !mStream->is_open() ) {
		mStream = (std::fstream*)AAssetManager_open( BaseWindow::androidState()->activity->assetManager, filename.c_str(), AASSET_MODE_STREAMING );
		mIsAsset = true;
	}
#elif GE_IOS
	mStream = new std::fstream( filename, std_mode );
	if ( !mStream->is_open() ) {
		delete mStream;
		mStream = _ge_iOSOpen( filename.c_str(), std_mode );
	}
#else
	mStream = new std::fstream( path, std_mode );
#endif
}


File::File( void* data, uint64_t len, MODE mode, bool copy_buffer, Instance* instance )
	: mType( BUFFER )
	, mMode( mode )
	, mCopiedBuffer( copy_buffer )
	, mInstance( instance ? instance : Instance::baseInstance() )
{
	if ( copy_buffer ) {
		mBuffer = ( unsigned char* )mInstance->Malloc( len, false );
		memcpy( mBuffer, data, len );
	} else {
		mBuffer = ( unsigned char* )data;
	}
	mBufferSize = len;
	mOffset = 0;

	std::stringstream path;
	path << "gemem://" << std::hex << mBuffer << ":" << std::dec << mBufferSize;
	mPath = path.str();
}


File::File( const File* other )
	: mType( other->mType )
	, mMode( other->mMode )
	, mStream( nullptr )
{
	fDebug( other );

	if ( mType == FILE ) {
		std::ios_base::openmode std_mode = std::ios_base::binary;

		if ( mMode & READ ) {
			std_mode |= std::ios_base::in;
		}
		if ( mMode & WRITE ) {
			std_mode |= std::ios_base::out;
		}

		mPath = other->mPath;
	#ifdef GE_ANDROID
		mIsAsset = false;
		mStream = new std::fstream( mPath, std_mode );
		if ( !mStream->is_open() ) {
			mStream = new std::fstream( std::string( BaseWindow::androidState()->activity->internalDataPath ) + "/" + mPath, std_mode );
		}
		if ( !mStream->is_open() ) {
			mStream = (std::fstream*)AAssetManager_open( BaseWindow::androidState()->activity->assetManager, mPath.c_str(), AASSET_MODE_STREAMING );
			mIsAsset = ( mStream != nullptr );
		}
	#elif GE_IOS
		mStream = new std::fstream( mPath, std_mode );
		if ( !mStream->is_open() ) {
			delete mStream;
			mStream = _ge_iOSOpen( mPath.c_str(), std_mode );
		}
	#else
		mStream = new std::fstream( mPath, std_mode );
	#endif
	}
}


File::~File()
{
	if ( mType == BUFFER ) {
		if ( mCopiedBuffer ) {
			mInstance->Free( mBuffer );
		}
	} else if ( mType == FILE ) {
#ifdef GE_ANDROID
		if ( mIsAsset and mStream ) {
			AAsset_close( (AAsset*)mStream );
		} else
#endif
		if ( mStream ) {
			if ( mStream->is_open() ) {
				mStream->close();
			}
			delete mStream;
		}
	}
}


bool File::isOpened()
{
	if ( mType == BUFFER ) {
		return true;
	} else if ( mType == FILE ) {
#ifdef GE_ANDROID
		if ( mIsAsset ) {
			return mStream != nullptr;
		} else
#endif
		{
			return mStream && mStream->is_open();
		}
	}
	return false;
}


void File::Rewind()
{
	if ( mType == BUFFER ) {
		mOffset = 0;
	} else if ( mType == FILE ) {
#ifdef GE_ANDROID
		if ( mIsAsset ) {
			AAsset_seek( (AAsset*)mStream, 0, 0 );
		} else
#endif
		{
			mStream->seekg( 0, std::ios_base::beg );
		}
	}
}


uint64_t File::Seek( int64_t ofs, DIR dir )
{
	if ( mType == BUFFER ) {
		if ( dir == BEGIN && ofs >= 0 && ofs < (int64_t)mBufferSize ) {
			mOffset = ofs;
		} else if ( dir == CURR && (int64_t)mOffset + ofs >= 0 && (int64_t)mOffset + ofs < (int64_t)mBufferSize ) {
			mOffset = mOffset + ofs;
		} else if ( dir == END && (int64_t)mBufferSize + ofs >= 0 && (int64_t)mBufferSize + ofs < (int64_t)mBufferSize ) {
			mOffset = mBufferSize + ofs;
		}
		return mOffset;
	}

	std::ios_base::seekdir std_dir = (std::ios_base::seekdir)-1;
	std_dir = ( dir == BEGIN ) ? std::ios_base::beg : std_dir;
	std_dir = ( dir == CURR ) ? std::ios_base::cur : std_dir;
	std_dir = ( dir == END ) ? std::ios_base::end : std_dir;
	if ( (int)std_dir != -1 ) {
#ifdef GE_ANDROID
		if ( mIsAsset ) {
			AAsset_seek( (AAsset*)mStream, ofs, (int)std_dir );
		} else
#endif
		{
			mStream->seekg( ofs, std_dir );
		}
	}
#ifdef GE_ANDROID
		if ( mIsAsset ) {
			return AAsset_seek( (AAsset*)mStream, 0, 1 );
		}
#endif
	return mStream->tellg();
}


uint64_t File::Read( void* buf, uint64_t len )
{
	uint64_t ret = 0;

	if ( mType == BUFFER ) {
		if ( mOffset + len >= mBufferSize ) {
			len = mBufferSize - mOffset;
		}
		memcpy( buf, &mBuffer[mOffset], len );
		ret = len;
	} else if ( mType == FILE ) {
#ifdef GE_ANDROID
		if ( mIsAsset ) {
			ret = AAsset_read( (AAsset*)mStream,(char*)buf, len );
		} else
#endif
		{
			mStream->read( (char*)buf, len );
			ret = mStream->gcount();
		}
	}

	return ret;
}


bool File::ReadLine( std::string& line )
{
	if ( mType == BUFFER ) {
		// TODO
	} else if ( mType == FILE ) {
#ifdef GE_ANDROID
		if ( mIsAsset ) {
			std::string reading = "";
			uint64_t ret = 0;
			char buf[129] = "";
			do {
				ret = Read( buf, 128 );
				buf[ret] = 0x0;
				reading = reading + std::string( buf );
				if ( strchr( buf, '\n' ) ) {
					size_t len = strchr( buf, '\n' ) - buf;
					Seek( -( ret - len ), File::CURR );
					break;
				}
			} while ( reading.find( "\n" ) < 0 && ret > 0 );
			line = reading.substr( 0, reading.find( "\n" ) + 1 );
		} else
#endif
		{
			std::getline( *mStream, line, '\n' );
			return !mStream->eof();
		}
	}
	return false;
}


std::string File::ReadLine()
{
	std::string ret;
	ReadLine( ret );
	return ret;
}


uint64_t File::Write( const void* buf, uint64_t len )
{
	uint64_t ret = 0;
	auto last = mStream->tellp();

	if ( mType == BUFFER ) {
		// TODO
	} else if ( mType == FILE ) {
		mStream->write( (char*)buf, len );
		ret = mStream->tellp() - last;
	}

	return ret;
}


uint64_t File::Write( const std::string& s )
{
	return Write( s.c_str(), s.length() );
}

} // namespace GE
