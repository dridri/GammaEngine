#include "AudioLoaderWav.h"
#include "File.h"
#include "Debug.h"

#define getIntBuffer( b, a ) ( (int)( (uint8_t)b[a] | (uint16_t)(b[a+1] << 8 ) | (uint32_t)(b[a+2] << 16 ) | (uint32_t)(b[a+3] << 24 ) ) )
#define getShortBuffer( b, a ) ( (short)( (uint8_t)b[a] | (uint16_t)( b[a+1] << 8 ) ) )

using namespace GE;


AudioLoader* AudioLoaderWav::NewInstance()
{
	return new AudioLoaderWav();
}


std::vector< std::string > AudioLoaderWav::contentPatterns()
{
	std::vector< std::string > ret;
	ret.emplace_back( "RIFF" );
	ret.emplace_back( "WAVE" );
	ret.emplace_back( "fmt " );
	return ret;
}


std::vector< std::string > AudioLoaderWav::extensions()
{
	std::vector< std::string > ret;
	ret.emplace_back( "wav" );
	ret.emplace_back( "riff" );
	ret.emplace_back( "aiff" );
	return ret;
}


void AudioLoaderWav::Rewind()
{
	mFile->Rewind();
	mEos = false;
}


void AudioLoaderWav::Load( Instance* instance, File* file, bool fullLoading )
{
	uint8_t header[44] = { 0 };
	mFile = file;
	mFile->Read( header, sizeof( header ) );

	mChannelsCount = getShortBuffer( header, 22 );
	mSampleRate = (unsigned short)getShortBuffer( header, 24 );
	mBps = getShortBuffer( header, 30 ) * 8;
	mDataSize = mFile->Seek( 0, File::END ) - sizeof( header );

	if ( fullLoading ) {
		mData = (uint16_t*)instance->Malloc( mDataSize );
		mFile->Seek( sizeof( header ), File::BEGIN );
		mFile->Read( mData, mDataSize );
	}
}


int32_t AudioLoaderWav::FillBuffer( uint16_t* buffer, uint32_t maxSize )
{
	int32_t ret = mFile->Read( buffer, maxSize );
	mDataOffset += ret;
	if ( ret == 0 ) {
		mEos = true;
	}
	return ret;
}
