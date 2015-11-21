#ifndef GE_SOUND_H
#define GE_SOUND_H

#include <string>
#include <vector>

#include "Instance.h"
#include "Time.h"
#include "MetaObject.h"
#include "AudioOutput.h"

namespace GE {

class Camera;
class File;
class AudioLoader;

class Sound : public MetaObject
{
public:
	Sound( const std::string& filename );
	Sound( File* file, const std::string& extension = "" );
	Sound( const Sound& other );
	~Sound();
	Sound& operator=( const Sound& other );

	void setAudioOutput( AudioOutput* output );

	bool isPlaying() const;
	uint32_t duration() const;

	void UpdateCamera( Camera* camera );
	void Play();
	void Stop();

	static AudioLoader* AddAudioLoader( AudioLoader* loader );

private:
	void Load( File* file, const std::string& extension, Instance* instance );

	AudioOutput* mOutput;
	AudioLoader* mSource;
	bool mSelfOutput;

	static std::vector< AudioLoader* > mAudioLoaders;
};


class AudioLoader
{
public:
	AudioLoader() : mFile( nullptr ), mSampleRate( 0 ), mBps( 0 ), mChannelsCount( 0 ), mData( nullptr ), mDataSize( 0 ), mEos( false ) { ; }
	virtual ~AudioLoader() { ; }
	virtual std::vector< std::string > contentPatterns() = 0;
	virtual std::vector< std::string > extensions() = 0;
	virtual AudioLoader* NewInstance() = 0;
	virtual void Load( Instance* instance, File* file, bool fullLoading ) = 0;
	virtual int32_t FillBuffer( uint16_t* buffer, uint32_t maxSize ) = 0;
	virtual void Rewind() = 0;
	uint32_t sampleRate() const { return mSampleRate; }
	uint32_t bps() const { return mBps; }
	uint32_t channelsCount() const { return mChannelsCount; }
	uint16_t* data() const { return mData; }
	uint32_t dataSize() const { return mDataSize; }
	bool eos() const { return mEos; }

protected:
	File* mFile;
	uint32_t mSampleRate;
	uint32_t mBps;
	uint32_t mChannelsCount;
	uint16_t* mData;
	uint32_t mDataSize;
	uint32_t mDataOffset;
	bool mEos;
};

}

#endif // GE_SOUND_H
