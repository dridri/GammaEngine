#ifndef AUDIOLOADERMP3_H
#define AUDIOLOADERMP3_H

#include "Sound.h"
#include <mad.h>

namespace GE {

class AudioLoaderMp3 : public AudioLoader
{
public:
	AudioLoaderMp3() : AudioLoader(), mUnderrunBufferSize( 0 ) { ; }
	virtual std::vector< std::string> contentPatterns();
	virtual std::vector< std::string > extensions();
	virtual AudioLoader* NewInstance();
	virtual void Load( Instance* instance, File* file, bool fullLoading );
	virtual int32_t FillBuffer( uint16_t* buffer, uint32_t maxSize );
	virtual void Rewind();

protected:
	int ErrorCheck();
	static std::string PrintFrameInfo( struct mad_header* Header );
	static signed short MadFixedToSshort( mad_fixed_t Fixed );
	uint32_t mFrameCount;
	struct mad_stream* mStream;
	struct mad_frame* mFrame;
	struct mad_synth* mSynth;
	mad_timer_t* mTimer;
	uint8_t mInputBuffer[1024 * 128];
	uint16_t mUnderrunBuffer[1152 * 4];
	uint32_t mUnderrunBufferSize;
};

};

#endif // AUDIOLOADERMP3_H
