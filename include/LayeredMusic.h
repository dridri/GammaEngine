#ifndef GE_LAYEREDMUSIC_H
#define GE_LAYEREDMUSIC_H

#include <mutex>
#include "Thread.h"
#include "Sound.h"

namespace GE
{

class LayeredMusic : protected Thread
{
public:
	LayeredMusic();
	~LayeredMusic();

	void Play();
	void Pause();
	void Stop();

	void setBPM( int bpm );
	void setVolume( uint32_t layerid, float vol );
	void PushSound( uint32_t layerid, Sound* sound, bool loop = true, bool overlap_curr = true );

protected:
	bool run();

private:
	class Layer {
	public:
		Layer( ) : output( nullptr ), sound( nullptr ), newsound( nullptr ), time_offset( 0 ), loop( false ), overlap( false ), volume( 1.0f ), iBuf( 0 ), nBuf( 0 ), cBuf( 0 ) {}
		void Feed( float beat_per_sec );
		AudioOutput* output;
		Sound* sound;
		Sound* newsound;
		uint32_t time_offset;
		bool loop;
		bool overlap;
		float volume;
	private:
		uint32_t iBuf;
		uint32_t nBuf;
		uint32_t cBuf;
	};

	std::vector< Layer > mLayers;
	float mBPS;
	std::mutex mMutex;
};
}

#endif // GE_LAYEREDMUSIC_H
