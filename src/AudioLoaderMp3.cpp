#include <limits.h>
#include "AudioLoaderMp3.h"
#include "File.h"
#include "Debug.h"

#define getIntBuffer( b, a ) ( (int)( (uint8_t)b[a] | (uint16_t)(b[a+1] << 8 ) | (uint32_t)(b[a+2] << 16 ) | (uint32_t)(b[a+3] << 24 ) ) )
#define getShortBuffer( b, a ) ( (short)( (uint8_t)b[a] | (uint16_t)( b[a+1] << 8 ) ) )

using namespace GE;


AudioLoader* AudioLoaderMp3::NewInstance()
{
	fDebug0();
	return new AudioLoaderMp3();
}


std::vector< std::string > AudioLoaderMp3::contentPatterns()
{
	std::vector< std::string > ret;
	ret.emplace_back( "ID3" );
	return ret;
}


std::vector< std::string > AudioLoaderMp3::extensions()
{
	std::vector< std::string > ret;
	ret.emplace_back( "mp3" );
	ret.emplace_back( "mp4" );
	ret.emplace_back( "m4a" );
	ret.emplace_back( "mpeg" );
	return ret;
}


void AudioLoaderMp3::Rewind()
{
	mUnderrunBufferSize = 0;
	mEos = false;
	mFrameCount = 2;

	mad_stream_init( mStream );
	mad_frame_init( mFrame );
	mad_synth_init( mSynth );
	mad_timer_reset( mTimer );

	mFile->Rewind();
	int sz = mFile->Read( mInputBuffer, sizeof( mInputBuffer ) );
	mad_stream_buffer( mStream, mInputBuffer, sz );
}


int AudioLoaderMp3::ErrorCheck()
{
	int dec = 0;
	if ( mStream->buffer == nullptr || mStream->error == MAD_ERROR_BUFLEN )
	{
		mStream->error = (mad_error)0;
		return 1;
	}
	if ( ( dec = mad_frame_decode( mFrame, mStream ) ) )
	{
		if ( MAD_RECOVERABLE ( mStream->error ) )
		{
			gDebug() << "libmad error: " << mStream->error << "  " << mad_stream_errorstr(mStream) << "\n";
			return 1;
		}
		else if ( mStream->error == MAD_ERROR_BUFLEN )
		{
			gDebug() << "libmad error: MAD_ERROR_BUFLEN\n";
			gDebug() << "end of stream\n";
			return -1;
		}
		else
		{
		}
	}
// 	gDebug() << "mad_frame_decode : " << dec << "\n";
	return 0;
}


void AudioLoaderMp3::Load( Instance* instance, File* file, bool fullLoading )
{
	fDebug( instance, file, fullLoading );

	uint8_t header[ 1024 * 128 ] = { 0 };
	mData = nullptr;
	mFrameCount = 0;
	mFile = new File ( file );
	mFile->Read( header, sizeof( header ) );
	mFile->Rewind();

	mStream = (struct mad_stream*)instance->Malloc( sizeof(struct mad_stream) );
	mFrame = (struct mad_frame*)instance->Malloc( sizeof(struct mad_frame) );
	mSynth = (struct mad_synth*)instance->Malloc( sizeof(struct mad_synth) );
	mTimer = (mad_timer_t*)instance->Malloc( sizeof(mad_timer_t) );

	mad_stream_init( mStream );
	mad_frame_init( mFrame );
	mad_synth_init( mSynth );
	mad_timer_reset( mTimer );

// 	MP3_TotalLength(mp3, &mp3->total_hours, &mp3->total_minuts, &mp3->total_seconds); TODO / TBD

	mad_stream_buffer( mStream, header, sizeof( header ) );
	while ( mFrameCount <= 2 )
	{
		int err = ErrorCheck();
		if ( err > 0 ) {
			continue;
		} else if ( err < 0 ) {
			break;
		}
		if ( mFrameCount == 1 ) {
			gDebug() << PrintFrameInfo( &mFrame->header ) << "\n";
			mSampleRate = mFrame->header.samplerate;
			mChannelsCount = MAD_NCHANNELS( &mFrame->header );
			mBps = 16;
		}
		mFrameCount++;
	}
	gDebug() << "1" << "\n";

	int sz = mFile->Read( mInputBuffer, sizeof( mInputBuffer ) );
	gDebug() << "2" << "\n";
	mad_stream_buffer( mStream, mInputBuffer, sz );
	gDebug() << "3" << "\n";

	if ( fullLoading ) {
		uint16_t buf[1152 * 2];
		int32_t ret = 0;
		while ( ( ret = FillBuffer( buf, sizeof( buf ) / sizeof( short ) ) ) >= 0 ) {
			ret *= 2;
			mData = (uint16_t*)instance->Realloc( mData, mDataSize + ret );
			memcpy( &mData[mDataSize / 2], buf, ret );
			mDataSize += ret;
		}
	}
	gDebug() << "4" << "\n";
}


int32_t AudioLoaderMp3::FillBuffer( uint16_t* buffer, uint32_t maxSize )
{
// 	fDebug( buffer, maxSize );
// 	gDebug() << "1\n";
	bool stop = false;
	uint32_t total = 0;

	if ( mUnderrunBufferSize > 0 ) {
		int sz = std::min( mUnderrunBufferSize, maxSize );
		memcpy( buffer, mUnderrunBuffer, sizeof(uint16_t) * sz );
		mUnderrunBufferSize -= sz;
		total += sz;
	}
// 	gDebug() << "2\n";

	while ( not stop and total < maxSize ) {
// 		gDebug() << "2.1\n";
		int err = ErrorCheck();
// 		gDebug() << "2.2\n";
		if ( err > 0 ) {
			return 0;
		} else if ( err < 0 ) {
			int left_data_sz = mStream->bufend - mStream->next_frame;
			memcpy( mInputBuffer, mStream->next_frame, left_data_sz );
			int sz = mFile->Read( mInputBuffer + left_data_sz, sizeof( mInputBuffer ) - left_data_sz );
			if ( sz == 0 ) {
				mEos = true;
				return -1;
			}
			mad_stream_buffer( mStream, mInputBuffer, sz + left_data_sz );
			return total;
		}
// 		gDebug() << "2.3\n";
		mFrameCount++;
		mad_synth_frame( mSynth, mFrame );
// 		gDebug() << "2.4\n";

		int32_t i = 0;
		for ( i = 0; i < mSynth->pcm.length; i++ ) {
// 			gDebug() << "2.4.1\n";
			signed short Sample;
			if ( total + i * 2 + 1 >= maxSize ) {
				mUnderrunBufferSize = ( mSynth->pcm.length - i ) * 2;
				for ( uint32_t j = i; j < mSynth->pcm.length; j++ ) {
					if ( MAD_NCHANNELS( &mFrame->header ) == 2 ) {
						mUnderrunBuffer[(j - i) * 2] = MadFixedToSshort( mSynth->pcm.samples[0][j] );
						mUnderrunBuffer[(j - i) * 2 + 1] = MadFixedToSshort( mSynth->pcm.samples[1][j] );
					} else {
						Sample = MadFixedToSshort( mSynth->pcm.samples[0][j] );
						mUnderrunBuffer[(j - i) * 2] = Sample;
						mUnderrunBuffer[(j - i) * 2 + 1] = Sample;
					}
				}
				stop = true;
				break;
			}
// 			gDebug() << "2.4.2 " << total << "+" << i*2 << " / " << maxSize << "\n";
			if ( MAD_NCHANNELS( &mFrame->header ) == 2 ) {
				buffer[total + i * 2] = MadFixedToSshort( mSynth->pcm.samples[0][i] );
				buffer[total + i * 2 + 1] = MadFixedToSshort( mSynth->pcm.samples[1][i] );
			} else {
				Sample = MadFixedToSshort( mSynth->pcm.samples[0][i] );
				buffer[total + i * 2] = Sample;
				buffer[total + i * 2 + 1] = Sample;
			}
// 			gDebug() << "2.4.3\n";
		}
// 		gDebug() << "2.5\n";
		total += i * 2;
	}
// 	gDebug() << "3 (" << total << ")\n";
	return total;
}


std::string AudioLoaderMp3::PrintFrameInfo( struct mad_header* Header )
{
	std::string Layer;
	std::string Mode;
	std::string Emphasis;

	switch ( Header->layer ) {
		case MAD_LAYER_I:
			Layer = "I";
			break;
		case MAD_LAYER_II:
			Layer = "II";
			break;
		case MAD_LAYER_III:
			Layer = "III";
			break;
		default:
			Layer = "(unexpected layer value)";
			break;
	}

	switch ( Header->mode ) {
		case MAD_MODE_SINGLE_CHANNEL:
			Mode = "single channel";
			break;
		case MAD_MODE_DUAL_CHANNEL:
			Mode = "dual channel";
			break;
		case MAD_MODE_JOINT_STEREO:
			Mode = "joint (MS/intensity) stereo";
			break;
		case MAD_MODE_STEREO:
			Mode = "normal LR stereo";
			break;
		default:
			Mode = "(unexpected mode value)";
			break;
	}

	switch ( Header->emphasis ) {
		case MAD_EMPHASIS_NONE:
			Emphasis = "no";
			break;
		case MAD_EMPHASIS_50_15_US:
			Emphasis = "50/15 us";
			break;
		case MAD_EMPHASIS_CCITT_J_17:
			Emphasis = "CCITT J.17";
			break;
		default:
			Emphasis = "(unexpected emphasis value)";
			break;
	}

	return std::to_string( Header->bitrate / 1000 ) + " kb/s audio MPEG layer " + Layer + " " + Mode + " " + std::to_string( Header->samplerate ) + "Hz";
}


signed short AudioLoaderMp3::MadFixedToSshort( mad_fixed_t Fixed )
{
	if ( Fixed >= MAD_F_ONE ) {
		return (SHRT_MAX);
	}
	if ( Fixed <= -MAD_F_ONE ) {
		return (-SHRT_MAX);
	}
	Fixed = Fixed >> ( MAD_F_FRACBITS - 15 );
	return ( (signed short) Fixed );
}
