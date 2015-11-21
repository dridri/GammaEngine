#include <algorithm>
#include "Music.h"
#include "File.h"
#include "Debug.h"
#include "AudioLoaderWav.h"
#include "AudioLoaderMp3.h"

using namespace GE;

std::vector< AudioLoader* > Music::mAudioLoaders = std::vector< AudioLoader* >();
static bool AudioLoaderFirstCall = true;

Music::Music( const std::string& filename )
	: MetaObject()
	, Thread()
	, mOutput( nullptr )
	, mSource( nullptr )
{
	File* file = new File( filename, File::READ );
	std::string extension = filename.substr( filename.rfind( "." ) + 1 );

	Load( file, extension, Instance::baseInstance() );

	delete file;
}


Music::Music( File* file, const std::string& extension )
	: MetaObject()
	, Thread()
	, mOutput( nullptr )
	, mSource( nullptr )
{
	Load( file, extension, Instance::baseInstance() );
}


Music::Music( const Music& other )
	: MetaObject()
	, Thread()
	, mOutput( nullptr )
	, mSource( nullptr )
{
}


Music::~Music()
{
	if ( mOutput ) {
		delete mOutput;
	}
	if ( mSource ) {
		delete mSource;
	}
}


Music& Music::operator=( const Music& other )
{
	*this = other;
	return *this;
}


void Music::Play()
{
	fDebug0();

	if ( mOutput ) {
		mOutput->Resume();
	}

	Thread::Start();
}


void Music::Stop()
{
	fDebug0();

	if ( mOutput ) {
		mOutput->Stop();
	}
	Thread::Pause();
	while ( Thread::running() ) {
		Time::Sleep( 10 );
	}

	if ( mOutput ) {
		delete mOutput;
		mOutput = nullptr;
	}

	if ( mSource ) {
		mSource->Rewind();
	}
}


bool Music::run()
{
// 	fDebug0();
	uint16_t* buffer = new uint16_t[1152 * 2 * 128];

	if ( !mOutput and mSource ) {
		mOutput = new AudioOutput( mSource->sampleRate(), mSource->bps(), mSource->channelsCount(), true );
	}
	if ( !mOutput or !mSource ) {
		delete buffer;
		return false;
	}

	int ret = mSource->FillBuffer( buffer, 1152 * 2 * 128 );
	if ( ret > 0 ) {
		mOutput->PushData( buffer, ret / sizeof(short) );
	} else if ( ret < 0 ) {
		gDebug() << "self pausing\n";
		Pause();
	}

	delete buffer;
	return true;
}


uint32_t Music::duration() const
{
	if ( mSource ) {
		uint32_t totalSamples = mSource->dataSize() / ( mSource->channelsCount() * mSource->bps() / 8 );
		return ( totalSamples / mSource->sampleRate() );
	}
	return 0;
}


bool Music::isPlaying() const
{
	if ( mSource and mSource->eos() ) {
		return false;
	}
	if ( mOutput ) {
		return mOutput->isPlaying();
	}
	return false;
}


void Music::Load( File* file, const std::string& extension, Instance* instance )
{
	AudioLoader* loader = nullptr;

	if ( AudioLoaderFirstCall ) {
		AddAudioLoader( new AudioLoaderWav() );
		AddAudioLoader( new AudioLoaderMp3() );
		AudioLoaderFirstCall = false;
	}

	char first_line[32];
	file->Rewind();
	file->Read( first_line, sizeof(first_line) );
	file->Rewind();

	for ( size_t i = 0; i < mAudioLoaders.size(); i++ ) {
		std::vector< std::string > patterns = mAudioLoaders.at(i)->contentPatterns();
		for ( size_t j = 0; j < patterns.size(); j++ ) {
			std::string test_case = patterns[j];
			for ( size_t k = 0; k < sizeof(first_line)-test_case.length(); k++ ) {
				if ( !memcmp( first_line + k, test_case.c_str(), test_case.length() ) ) {
					loader = mAudioLoaders.at(i);
					break;
				}
			}
		}
	}

	if ( !loader && extension.length() > 0 ) {
		for ( size_t i = 0; i < mAudioLoaders.size(); i++ ) {
			std::vector< std::string > extensions = mAudioLoaders.at(i)->extensions();
			for ( size_t j = 0; j < extensions.size(); j++ ) {
				std::string test_case = extensions[j];
				std::transform( test_case.begin(), test_case.end(), test_case.begin(), ::tolower );
				if ( extension.find( test_case ) == 0 ) {
					loader = mAudioLoaders.at(i);
					break;
				}
			}
		}
	}

	if ( loader ) {
		mSource = loader->NewInstance();
		mSource->Load( instance, file, false );
	}
}


AudioLoader* Music::AddAudioLoader( AudioLoader* loader )
{
	mAudioLoaders.insert( mAudioLoaders.begin(), loader );
	return loader;
}
