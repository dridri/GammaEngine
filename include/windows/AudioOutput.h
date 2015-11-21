#ifndef GE_AUDIOOUTPUT_H
#define GE_AUDIOOUTPUT_H

#include <vector>
#include <string>

#include "Time.h"

namespace GE {

class AudioOutput : public GE::Time
{
public:
	typedef enum {
		Front_Left = 1,
		Front_Right = 2
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
};
}

#endif // GE_AUDIOOUTPUT_H
