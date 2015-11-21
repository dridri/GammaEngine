#ifndef GE_MUSIC_H
#define GE_MUSIC_H

#include "Sound.h"
#include "Thread.h"

namespace GE {

class Music : public MetaObject, protected Thread
{
public:
	Music( const std::string& filename );
	Music( File* file, const std::string& extension = "" );
	Music( const Music& other );
	~Music();
	Music& operator=( const Music& other );

	bool isPlaying() const;
	uint32_t duration() const;

	void Play();
	void Stop();

	static AudioLoader* AddAudioLoader( AudioLoader* loader );

protected:
	virtual bool run();

private:
	void Load( File* file, const std::string& extension, Instance* instance );

	AudioOutput* mOutput;
	AudioLoader* mSource;

	static std::vector< AudioLoader* > mAudioLoaders;
};

}

#endif // GE_MUSIC_H
