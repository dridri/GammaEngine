#include <stdlib.h>
#include "windows/AudioOutput.h"
#include "Thread.h"
#include "Debug.h"

using namespace GE;

AudioOutput::AudioOutput( uint32_t sample_rate, uint32_t bps, uint32_t input_channels, bool blocking, int speakers )
	: Time()
{
	fDebug( sample_rate, bps, input_channels, blocking, speakers );
}


AudioOutput::~AudioOutput()
{
}


std::vector< std::pair< int, std::string > > AudioOutput::DevicesList()
{
	std::vector< std::pair< int, std::string > > ret;
	return ret;
}


void AudioOutput::PushData( uint16_t* data, uint32_t size )
{
	fDebug( data, size );
}


void AudioOutput::Stop()
{
}


void AudioOutput::Pause()
{
}


void AudioOutput::Resume()
{
}


bool AudioOutput::isPlaying() const
{
	return true;
}
