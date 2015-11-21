#include <stdlib.h>
#include "android/AudioOutput.h"
#include "Thread.h"
#include "Debug.h"

#define ASSERT( result ) \
	if ( result != SL_RESULT_SUCCESS ) { gDebug() << "Assertion failed at line " << __LINE__ << " ! : " << result << "\n"; exit(0); }

// #define ASSERT( result ) \
	if ( result != SL_RESULT_SUCCESS ) { gDebug() << "Assertion failed at line " << __LINE__ << " ! : " << result << "\n"; exit(0); } else { gDebug() << __LINE__ << " Ok\n"; }

using namespace GE;

void AudioOutput::sBqPlayerCallback( SLAndroidSimpleBufferQueueItf bq, void* p )
{
// 	fDebug( bq, p );
	((AudioOutput*)p)->mOutlock++;
}


void AudioOutput::sPlayerCallback( SLPlayItf caller, void* p, SLuint32 event )
{
// 	fDebug( caller, p, event );
	if ( event == SL_PLAYEVENT_HEADATEND ) {
// 		((AudioOutput*)p)->mOutlock++;
	}
}


void AudioOutput::WaitThreadLock()
{
	while ( mOutlock == mLastOutlock ) {
		Time::Sleep( 1 );
	}
	mLastOutlock = mOutlock;
}


AudioOutput::AudioOutput( uint32_t sample_rate, uint32_t bps, uint32_t input_channels, bool blocking, int speakers )
	: Time()
{
	fDebug( sample_rate, bps, input_channels, blocking, speakers );

	SLresult result;

	mOutchannels = input_channels;
	mSr = sample_rate;
	mOutlock = 0;
	mLastOutlock = 0;
// 	mOutBufSamples = 8192;
// 	mOutBuf = (short*)Instance::baseInstance()->Malloc( 2 * mOutBufSamples * sizeof(short) );
// 	mOutputBuffer[0] = &mOutBuf[0];
// 	mOutputBuffer[1] = &mOutBuf[mOutBufSamples];

// 	mCurrentOutputIndex = 0;
// 	mCurrentOutputBuffer = 0;
// 	mCpos = 0;
// 	mBlkpos = 0;
	
	// Create Engine
	result = slCreateEngine( &mEngineObject, 0, nullptr, 0, nullptr, nullptr );
	ASSERT( result );
	result = (*mEngineObject)->Realize( mEngineObject, SL_BOOLEAN_FALSE );
	ASSERT( result );
	result = (*mEngineObject)->GetInterface( mEngineObject, SL_IID_ENGINE, &mEngine );
	ASSERT( result );

	// Open Output Mix
	const SLInterfaceID ids[] = { SL_IID_VOLUME };
	const SLboolean req[] = { SL_BOOLEAN_FALSE };
	result = (*mEngine)->CreateOutputMix( mEngine, &mOutputMixObject, 1, ids, req );
	ASSERT( result );

	result = (*mOutputMixObject)->Realize( mOutputMixObject, SL_BOOLEAN_FALSE );
	ASSERT( result );
	
	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2 };
	SLDataFormat_PCM format_pcm = { SL_DATAFORMAT_PCM, 2, sample_rate*1000, SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16, (uint32_t)speakers, SL_BYTEORDER_LITTLEENDIAN };

	SLDataSource audioSrc = { &loc_bufq, &format_pcm };
	SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX, mOutputMixObject };
	SLDataSink audioSnk = { &loc_outmix, nullptr };

	const SLInterfaceID ids1[] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
	const SLboolean req1[] = { SL_BOOLEAN_TRUE };
	result = (*mEngine)->CreateAudioPlayer( mEngine, &mBqPlayerObject, &audioSrc, &audioSnk, 1, ids1, req1 );
	ASSERT( result );
	
	result = (*mBqPlayerObject)->Realize( mBqPlayerObject, SL_BOOLEAN_FALSE );
	ASSERT( result );

	result = (*mBqPlayerObject)->GetInterface( mBqPlayerObject, SL_IID_PLAY, &mBqPlayerPlay );
	ASSERT( result );

	result = (*mBqPlayerObject)->GetInterface( mBqPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &mBqPlayerBufferQueue );
	ASSERT( result );

	result = (*mBqPlayerBufferQueue)->RegisterCallback( mBqPlayerBufferQueue, sBqPlayerCallback, this );
	ASSERT( result );

	result = (*mBqPlayerPlay)->RegisterCallback( mBqPlayerPlay, sPlayerCallback, this );
	ASSERT( result );

	result = (*mBqPlayerPlay)->SetCallbackEventsMask( mBqPlayerPlay, SL_PLAYEVENT_HEADATEND | SL_PLAYEVENT_HEADSTALLED );
	ASSERT( result );

	mOutlock++;
// 	result = (*mBqPlayerPlay)->SetPlayState( mBqPlayerPlay, SL_PLAYSTATE_PLAYING );
// 	ASSERT( result );
}


AudioOutput::~AudioOutput()
{
	if ( mBqPlayerPlay != nullptr ) {
		(*mBqPlayerPlay)->SetPlayState( mBqPlayerPlay, SL_PLAYSTATE_STOPPED );
		(*mBqPlayerPlay)->SetMarkerPosition( mBqPlayerPlay, 0 );
	}

	if ( mBqPlayerBufferQueue != nullptr ) {
		(*mBqPlayerBufferQueue)->Clear( mBqPlayerBufferQueue );
	}
	if ( mBqPlayerObject != nullptr ) {
		(*mBqPlayerObject)->AbortAsyncOperation( mBqPlayerObject );
		(*mBqPlayerObject)->Destroy( mBqPlayerObject );
		mBqPlayerObject = nullptr;
		mBqPlayerPlay = nullptr;
		mBqPlayerBufferQueue = nullptr;
		mBqPlayerEffectSend = nullptr;
	}

	if ( mOutputMixObject != nullptr ) {
		(*mOutputMixObject)->Destroy( mOutputMixObject );
		mOutputMixObject = nullptr;
	}

	if ( mEngineObject != nullptr ) {
		(*mEngineObject)->Destroy( mEngineObject );
		mEngineObject = nullptr;
		mEngine = nullptr;
	}

	for ( auto buf : mQueue ) {
		delete buf;
	}
	mQueue.clear();
}


std::vector< std::pair< int, std::string > > AudioOutput::DevicesList()
{
	fDebug0();
	std::vector< std::pair< int, std::string > > ret;
/*	UNSUPPORTED BY ANDROID (LOL)
	SLresult result;
	SLObjectItf engineObject;
	SLAudioIODeviceCapabilitiesItf audioIODeviceCapabilities;
	SLAudioOutputDescriptor audioOutputDescriptor;
	int32_t numOutputs;
	uint32_t outputDeviceIDs[64];

	result = slCreateEngine( &engineObject, 0, nullptr, 0, nullptr, nullptr );
	ASSERT( result );
	result = (*engineObject)->Realize( engineObject, SL_BOOLEAN_FALSE );
	ASSERT( result );
	result = (*engineObject)->GetInterface( engineObject, SL_IID_AUDIOIODEVICECAPABILITIES, &audioIODeviceCapabilities );
	ASSERT( result );

	result = (*audioIODeviceCapabilities)->GetAvailableAudioOutputs( audioIODeviceCapabilities, &numOutputs, outputDeviceIDs );
	ASSERT( result );

	for ( int32_t i = 0; i < numOutputs; i++ ) {
		result = (*audioIODeviceCapabilities)->QueryAudioOutputCapabilities( audioIODeviceCapabilities, outputDeviceIDs[i], &audioOutputDescriptor );
		ret.push_back( std::make_pair( outputDeviceIDs[i], (char*)audioOutputDescriptor.pDeviceName ) );
		ASSERT( result );
	}

	(*engineObject)->Destroy( engineObject );
*/
	return ret;
}


void AudioOutput::PushData( uint16_t* data, uint32_t size )
{
// 	fDebug( data, size );

	SLAndroidSimpleBufferQueueState state;
	short* buf = new short[ size * 4 ];
	memcpy( buf, data, size * 2 * sizeof( short ) );

	(*mBqPlayerBufferQueue)->GetState( mBqPlayerBufferQueue, &state );

	if ( mQueue.size() > 0 ) {
		WaitThreadLock();
	}
	if ( mQueue.size() > state.count ) {
		delete mQueue.front();
		mQueue.pop_front();
	}
	(*mBqPlayerBufferQueue)->Enqueue( mBqPlayerBufferQueue, buf, size * 2 * sizeof(short) );

	uint32_t player_state;
	(*mBqPlayerPlay)->GetPlayState( mBqPlayerPlay, &player_state );
	if ( player_state != SL_PLAYSTATE_PLAYING ) {
		(*mBqPlayerPlay)->SetPlayState( mBqPlayerPlay, SL_PLAYSTATE_PLAYING );
	}

	mTime += (double) size / ( mSr * mOutchannels );
	mQueue.push_back( buf );
}


void AudioOutput::Stop()
{
	(*mBqPlayerPlay)->SetPlayState( mBqPlayerPlay, SL_PLAYSTATE_STOPPED );
// 	(*mBqPlayerPlay)->SetPlayState( mBqPlayerPlay, SL_PLAYSTATE_PAUSED );
// 	(*mBqPlayerPlay)->SetMarkerPosition( mBqPlayerPlay, 0 );
// 	(*mBqPlayerBufferQueue)->Clear( mBqPlayerBufferQueue );
// 	(*mBqPlayerObject)->AbortAsyncOperation( mBqPlayerObject );
// 	for ( auto buf : mQueue ) {
// 		delete buf;
// 	}
// 	mQueue.clear();
	mOutlock = 1;
	mLastOutlock = 0;
}


void AudioOutput::Pause()
{
	(*mBqPlayerPlay)->SetPlayState( mBqPlayerPlay, SL_PLAYSTATE_PAUSED );
}


void AudioOutput::Resume()
{
	(*mBqPlayerPlay)->SetPlayState( mBqPlayerPlay, SL_PLAYSTATE_PLAYING );
}


bool AudioOutput::isPlaying() const
{
	uint32_t state;
	(*mBqPlayerPlay)->GetPlayState( mBqPlayerPlay, &state );
	return state & SL_PLAYSTATE_PLAYING;
}
