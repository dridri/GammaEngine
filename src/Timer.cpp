#include "include/Time.h"
#include "Timer.h"

using namespace GE;

Timer::Timer()
	: mRunning( false )
	, mPaused( false )
	, mStart( 0 )
	, mPauseTime( 0 )
	, mPause( 0 )
{
}


void Timer::Start()
{
	mRunning = true;
	mStart = Time::GetTick();
}


void Timer::Pause()
{
	if ( mRunning and not mPaused ) {
		mPaused = true;
		mPauseTime = Time::GetTick();
	}
}


void Timer::Resume()
{
	if ( mRunning and mPaused ) {
		mPaused = false;
		mPause += Time::GetTick() - mPauseTime;
	}
}


void Timer::Stop()
{
	mRunning = false;
}


uint64_t Timer::ellapsed()
{
	return ( Time::GetTick() - mStart ) - mPause;
}


bool Timer::isRunning()
{
	return mRunning;
}
