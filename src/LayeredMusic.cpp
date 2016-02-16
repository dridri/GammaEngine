#include "Debug.h"
#include "LayeredMusic.h"

using namespace GE;

LayeredMusic::LayeredMusic()
	: Thread()
	, mLayers( decltype(mLayers)() )
	, mBPS( 0.0f )
{
}


LayeredMusic::~LayeredMusic()
{
	for ( Layer& layer : mLayers ) {
		delete layer.output;
	}
}


void LayeredMusic::Play()
{
	Start();
}


void LayeredMusic::Pause()
{
}


void LayeredMusic::Stop()
{
}


void LayeredMusic::setBPM( int bpm )
{
	mBPS = ( (float)bpm ) / 60.0f;
}


void LayeredMusic::setVolume( uint32_t layerid, float vol )
{
	mMutex.lock();

	if ( layerid >= mLayers.size() ) {
		mLayers.push_back( Layer() );
	}
	Layer& layer = mLayers.at( layerid );
	layer.volume = vol;

	if ( layer.output ) {
		layer.output->setVolume( layer.volume );
	}

	mMutex.unlock();
}


bool LayeredMusic::run()
{
	for ( Layer& layer : mLayers ) {
		mMutex.lock();
		if ( layer.output ) {
			layer.Feed( mBPS );
		}
		mMutex.unlock();
	}

	usleep( 1000 * 10 );
	return true;
}


void LayeredMusic::Layer::Feed( float beat_per_sec )
{
	if ( newsound ){
		sound = newsound;
		newsound = nullptr;
		nBuf = sound->dataSize() / ( sound->channelsCount() * ( sound->bps() / 8 ) );
		if ( beat_per_sec == 0 ) {
			cBuf = ( nBuf > 16384 ) ? 16384 : cBuf;
		} else {
			cBuf = (int)( (float)sound->sampleRate() / beat_per_sec );
		}
	}
	if ( output->samplesToWrite() <= cBuf * 0.25f ) {
		if ( iBuf + cBuf >= nBuf ) {
			if ( loop ) {
// 				output->PushData( sound->data() + iBuf * sound->channelsCount(), nBuf - iBuf );
// 				output->PushData( sound->data() + ( nBuf - iBuf ) * sound->channelsCount(), cBuf - ( nBuf - iBuf ) );
// 				iBuf = ( iBuf + cBuf ) % nBuf;
				output->PushData( sound->data() + iBuf * sound->channelsCount(), nBuf - iBuf );
				output->PushData( sound->data(), cBuf );
				iBuf = cBuf;
			} else {
			}
		} else {
			output->PushData( sound->data() + iBuf * sound->channelsCount(), cBuf );
			iBuf = ( iBuf + cBuf ) % nBuf;
		}
	}
}


void LayeredMusic::PushSound( uint32_t layerid, Sound* sound, bool loop, bool overlap_curr )
{
	fDebug( layerid, sound, loop, overlap_curr );
	mMutex.lock();

	if ( layerid >= mLayers.size() ) {
		mLayers.push_back( Layer() );
	}
	Layer& layer = mLayers.at( layerid );

	if ( !layer.output ) {
		layer.output = new AudioOutput( sound->sampleRate(), sound->bps(), sound->channelsCount() );
		layer.output->setVolume( layer.volume );
	}
	layer.loop = loop;
	layer.overlap = overlap_curr;
	layer.newsound = sound;

	mMutex.unlock();
}
