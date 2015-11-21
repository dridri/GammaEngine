#ifndef GE_AUDIOOUTPUT_H
#define GE_AUDIOOUTPUT_H

#include <list>
#include <vector>
#include <alsa/asoundlib.h>
#include "Time.h"
#include "Thread.h"

namespace GE {

class AudioOutputThread;

class AudioOutput : public GE::Time
{
public:
	typedef enum {
		Front_Left,
		Front_Right
	} Speaker;
	AudioOutput( uint32_t sample_rate, uint32_t bps, uint32_t input_channels, bool blocking = false, int speakers = Front_Left | Front_Right );
	~AudioOutput();

	void Stop();
	void Pause();
	void Resume();

	void PushData( uint16_t* data, uint32_t size );

	bool isPlaying() const;

	static std::vector< std::pair< int, std::string > > DevicesList();

private:
	snd_pcm_t* mHandle;
	uint16_t* mBuffer;
	int mBufferSize;
	int mSamplesSize;
	AudioOutputThread* mThread;
};


class AudioOutputThread : public Thread
{
public:
	AudioOutputThread( snd_pcm_t* h ) : Thread(), mHandle( h ), mPlaying( false ) { ; }
	~AudioOutputThread(){}
	virtual bool run();
	void PushData( uint16_t* data, uint32_t size );
	bool isPlaying() const { return mPlaying; }

protected:
	snd_pcm_t* mHandle;
	bool mPlaying;
	std::list< std::pair< uint16_t*, uint32_t > > mQueue;
};


}

#endif // GE_AUDIOOUTPUT_H
