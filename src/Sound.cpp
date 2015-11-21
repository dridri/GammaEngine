#include <algorithm>
#include "Sound.h"
#include "File.h"
#include "Debug.h"
#include "AudioLoaderWav.h"
#include "AudioLoaderMp3.h"

using namespace GE;

std::vector< AudioLoader* > Sound::mAudioLoaders = std::vector< AudioLoader* >();
static bool AudioLoaderFirstCall = true;

Sound::Sound( const std::string& filename )
	: MetaObject()
	, mOutput( nullptr )
	, mSource( nullptr )
	, mSelfOutput( false )
{
	File* file = new File( filename, File::READ );
	std::string extension = filename.substr( filename.rfind( "." ) + 1 );

	Load( file, extension, Instance::baseInstance() );

	delete file;
}


Sound::Sound( File* file, const std::string& extension )
	: MetaObject()
	, mOutput( nullptr )
	, mSource( nullptr )
	, mSelfOutput( false )
{
	Load( file, extension, Instance::baseInstance() );
}


Sound::Sound( const Sound& other )
	: MetaObject()
	, mOutput( nullptr )
	, mSource( nullptr )
	, mSelfOutput( false )
{
}


Sound::~Sound()
{
	if ( mSelfOutput and mOutput ) {
		delete mOutput;
	}
	if ( mSource ) {
		delete mSource;
	}
}


Sound& Sound::operator=( const Sound& other )
{
	*this = other;
	return *this;
}


void Sound::Play()
{
	if ( !mOutput and mSource ) {
		mSelfOutput = true;
		mOutput = new AudioOutput( mSource->sampleRate(), mSource->bps(), mSource->channelsCount() );
	}
	mOutput->PushData( mSource->data(), mSource->dataSize() / ( mSource->channelsCount() * mSource->bps() / 8 ) );
}


void Sound::Stop()
{
	if ( mOutput ) {
		// TODO
	}
}


void Sound::UpdateCamera( Camera* camera )
{

}


uint32_t Sound::duration() const
{
	if ( mSource ) {
		uint32_t totalSamples = mSource->dataSize() / ( mSource->channelsCount() * mSource->bps() / 8 );
		return ( totalSamples / mSource->sampleRate() );
	}
	return 0;
}


bool Sound::isPlaying() const
{
	if ( mSource and mSource->eos() ) {
		return false;
	}
	if ( mOutput ) {
		return mOutput->isPlaying();
	}
	return false;
}


void Sound::Load( File* file, const std::string& extension, Instance* instance )
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
		mSource->Load( instance, file, true );
	}
}


AudioLoader* Sound::AddAudioLoader( AudioLoader* loader )
{
	mAudioLoaders.insert( mAudioLoaders.begin(), loader );
	return loader;
}
