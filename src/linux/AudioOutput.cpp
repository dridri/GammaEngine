#include "linux/AudioOutput.h"
#include "Thread.h"
#include "Debug.h"

using namespace GE;

AudioOutput::AudioOutput( uint32_t sample_rate, uint32_t bps, uint32_t input_channels, bool blocking, int speakers )
	: Time()
	, mThread( nullptr )
{
	int err, dir;
	snd_pcm_hw_params_t* hwparams;
	snd_pcm_sw_params_t* swparams;

	if ( ( err = snd_pcm_open( &mHandle, "default", SND_PCM_STREAM_PLAYBACK, 0/*blocking ? 0 : SND_PCM_NONBLOCK*/ ) ) < 0 ) {
		gDebug() << "Can't open default PCM : " << snd_strerror(err) << "\n";
		return;
	}

	mSamplesSize = 1152*32;
	mBufferSize = input_channels * sizeof(short) * mSamplesSize;

	uint32_t rrate = sample_rate;
// 	int buffer_time = 500000;
// 	int period_time = 100000;
	snd_pcm_uframes_t size = 0;
	snd_pcm_sframes_t buffer_size = 0;
	snd_pcm_sframes_t period_size = 0;

	snd_pcm_hw_params_malloc( &hwparams );
	snd_pcm_sw_params_malloc( &swparams );


	err = snd_pcm_hw_params_any( mHandle, hwparams );
	if ( err < 0 ) {
		gDebug() << "Broken configuration for playback: no configurations available: " << snd_strerror(err) << "\n";
		snd_pcm_close( mHandle );
		mHandle = nullptr;
		return;
	}
	err = snd_pcm_hw_params_set_rate_resample( mHandle, hwparams, 1 );
	if ( err < 0 ) {
		gDebug() << "Resampling setup failed for playback: " << snd_strerror(err) << "\n";
		snd_pcm_close( mHandle );
		mHandle = nullptr;
		return;
	}
	err = snd_pcm_hw_params_set_access( mHandle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED );
	if ( err < 0 ) {
		gDebug() << "Access type not available for playback: " << snd_strerror(err) << "\n";
		snd_pcm_close( mHandle );
		mHandle = nullptr;
		return;
	}
	err = snd_pcm_hw_params_set_format( mHandle, hwparams, SND_PCM_FORMAT_S16  );
	if ( err < 0 ) {
		gDebug() << "Sample format not available for playback: " << snd_strerror(err) << "\n";
		snd_pcm_close( mHandle );
		mHandle = nullptr;
		return;
	}
	err = snd_pcm_hw_params_set_channels( mHandle, hwparams, input_channels );
	if ( err < 0 ) {
		gDebug() << "Channels count (" << input_channels << ") not available for playbacks: " << snd_strerror(err) << "\n";
		snd_pcm_close( mHandle );
		mHandle = nullptr;
		return;
	}
	err = snd_pcm_hw_params_set_rate_near( mHandle, hwparams, &rrate, 0 );
	if ( err < 0 ) {
		gDebug() << "Rate " << sample_rate << "Hz not available for playback: " << snd_strerror(err) << "\n";
		snd_pcm_close( mHandle );
		mHandle = nullptr;
		return;
	}
	if ( rrate != sample_rate ) {
		gDebug() << "Rate doesn't match (requested " << sample_rate << "Hz, get " << rrate << "Hz)" << "\n";
		snd_pcm_close( mHandle );
		mHandle = nullptr;
		return;
	}
	err = snd_pcm_hw_params_set_buffer_size_near( mHandle, hwparams, (snd_pcm_uframes_t*)&mSamplesSize );
	if ( err < 0 ) {
		gDebug() << "Unable to set buffer size for playback: " << snd_strerror(err) << "\n";
		snd_pcm_close( mHandle );
		mHandle = nullptr;
		return;
	}

	err = snd_pcm_hw_params( mHandle, hwparams );
	if ( err < 0 ) {
		gDebug() << "Unable to set hw hwparams for playback:" << snd_strerror(err) << "\n";
		snd_pcm_close( mHandle );
		mHandle = nullptr;
		return;
	}

	if ( not blocking ) {
		mThread = new AudioOutputThread( mHandle );
		mThread->Start();
	}

	mBuffer = (uint16_t*)Instance::baseInstance()->Malloc( mBufferSize );
	snd_pcm_start( mHandle );
}


AudioOutput::~AudioOutput()
{
}


std::vector< std::pair< int, std::string > > AudioOutput::DevicesList()
{
	std::vector< std::pair< int, std::string > > ret;
	int err;
	int cardId = -1;
	snd_ctl_card_info_t* cardInfo = nullptr;
	char str[64] = "";
	snd_ctl_t* cardHandle = nullptr;

	snd_ctl_card_info_alloca( &cardInfo );
	while ( ( err = snd_card_next( &cardId ) ) >= 0 and cardId >= 0 ) {
		sprintf( str, "hw:%d", cardId );
		if ( ( err = snd_ctl_open( &cardHandle, str, 0 ) ) < 0 ) {
			gDebug() << "Can't open card " << cardId << ": " << snd_strerror( err ) << "\n";
			break;
		}
		if ( ( err = snd_ctl_card_info( cardHandle, cardInfo ) ) < 0 ) {
			gDebug() << "Can't get info for card " << cardId << ": " << snd_strerror( err ) << "\n";
		} else {
			ret.push_back( std::make_pair( cardId, snd_ctl_card_info_get_name( cardInfo ) ) );
		}
		snd_ctl_close( cardHandle );
	}
	return ret;
}


void AudioOutput::PushData( uint16_t* data, uint32_t size )
{
	if ( mHandle ) {
		if ( mThread ) {
			mThread->PushData( data, size );
		} else {
			fDebug( data, size );
			snd_pcm_writei( mHandle, data, size );
		}
	}
}


bool AudioOutputThread::run()
{
	while ( mQueue.size() > 0 ) {
		std::pair< uint16_t*, uint32_t > q = mQueue.front();
		mPlaying = true;
		snd_pcm_writei( mHandle, q.first, q.second );
		mQueue.pop_front();
	}
	mPlaying = false;
	// TODO : return ( this->Pause() )
	return true;
}


void AudioOutputThread::PushData( uint16_t* data, uint32_t size )
{
	mQueue.push_back( std::make_pair( data, size ) );
	// TODO : wake-up thread
}


void AudioOutput::Pause()
{
	snd_pcm_pause( mHandle, true );
}


void AudioOutput::Resume()
{
	snd_pcm_pause( mHandle, false );
	if ( snd_pcm_state( mHandle ) != SND_PCM_STATE_RUNNING ) {
		snd_pcm_start( mHandle );
	}
}


void AudioOutput::Stop()
{
	snd_pcm_drop( mHandle );
	// TODO
}


bool AudioOutput::isPlaying() const
{
	gDebug() << "ret : " << snd_pcm_avail( mHandle ) << "\n";
	gDebug() << "state : " << snd_pcm_state( mHandle ) << "\n";
	if ( not mHandle or ( mThread and not mThread->isPlaying() ) or snd_pcm_state( mHandle ) != SND_PCM_STATE_RUNNING ) {
		return false;
	}
	return true;
}
