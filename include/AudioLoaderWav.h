#ifndef AUDIOLOADERWAV_H
#define AUDIOLOADERWAV_H

#include "Sound.h"

namespace GE {

class AudioLoaderWav : public AudioLoader
{
public:
	AudioLoaderWav() : AudioLoader() { ; }
	virtual std::vector< std::string> contentPatterns();
	virtual std::vector< std::string > extensions();
	virtual AudioLoader* NewInstance();
	virtual void Load( Instance* instance, File* file, bool fullLoading );
	virtual int32_t FillBuffer( uint16_t* buffer, uint32_t maxSize );
	virtual void Rewind();

private:
};

};

#endif // AUDIOLOADERWAV_H
