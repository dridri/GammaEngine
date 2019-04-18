/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2015  Adrien Aubry <dridri85@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cmath>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdint.h>
#include "include/Time.h"

#ifdef GE_WIN32
	#include <windows.h>
#endif

using namespace GE;


int64_t Time::sStartTime = 0;
// int64_t Time::sTime = 0;
decltype(Time::sTime) Time::sTime = decltype(Time::sTime)();
int64_t Time::sPause = 0;
// double Time::sDt = 0.0;
decltype(Time::sDt) Time::sDt = decltype(Time::sDt)();


Time::Time()
	: mDt( 0.0 )
	, mSlowDt( 0.0 )
	, mParent( nullptr )
{
	if ( sTime.find( (uint64_t)pthread_self() ) == sTime.end() ) {
		sTime.insert( std::make_pair( (uint64_t)pthread_self(), GetTick() ) );
	}
}


void Time::IncreasePause( int64_t t )
{
	sPause += t;
}


void Time::setTimeParent( Time* t )
{
	mParent = t;
}


void Time::setTime( int64_t t )
{
// 	int64_t dt = t - sTime;
	int64_t dt = t - sTime.at( (uint64_t)pthread_self() );
	sDt[ (uint64_t)pthread_self() ] = ( (double)dt ) / 1000.0;
// 	sTime = t;
	sTime[ (uint64_t)pthread_self() ] = t;
}


void Time::GlobalSync()
{
	if ( sTime[ (uint64_t)pthread_self() ] == 0 ) {
		sTime[ (uint64_t)pthread_self() ] = GetTick();
	}

	int64_t dt = GetTick() - sTime[ (uint64_t)pthread_self() ];
	sDt[ (uint64_t)pthread_self() ] = ( (double)dt ) / 1000.0;
	sTime[ (uint64_t)pthread_self() ] = GetTick();
}


double Time::Delta()
{
	return sDt[ (uint64_t)pthread_self() ];
}


double Time::Sync()
{
	if ( mParent ) {
		return mParent->mDt;
	}

// 	uint32_t nTime = GetTick();

	if ( sTime[ (uint64_t)pthread_self() ] == 0 ) {
		GlobalSync();
	}

	mDt = sDt[ (uint64_t)pthread_self() ];
	return mDt;

// 	uint32_t dt = GetTick() - sTime;
// 	return ( (float)dt ) / 1000.0f;
}


double Time::SlowSync( double min )
{
	mSlowDt += Sync();
	if ( std::abs( mSlowDt ) >= min ) {
		mDt = mSlowDt;
		mSlowDt = 0.0;
		return mDt;
	}
	return 0.0;
}


uint64_t Time::GetTick()
{
	if ( sStartTime == 0 ) {
	#ifdef GE_WIN32
		sStartTime = timeGetTime();
	#elif GE_IOS
		struct timeval cTime;
		gettimeofday( &cTime, 0 );
		sStartTime = ( cTime.tv_sec * 1000 ) + ( cTime.tv_usec / 1000 );
	#else
		struct timespec now;
		clock_gettime( CLOCK_MONOTONIC, &now );
		sStartTime = now.tv_sec * 1000 + now.tv_nsec / 1000000;
	#endif
	}

#ifdef GE_WIN32
	return timeGetTime() - sStartTime - sPause;
#elif GE_IOS
	struct timeval cTime;
	gettimeofday( &cTime, 0 );
	return ( cTime.tv_sec * 1000 ) + ( cTime.tv_usec / 1000 ) - sStartTime - sPause;
#else
	struct timespec now;
	clock_gettime( CLOCK_MONOTONIC, &now );
	return now.tv_sec * 1000 + now.tv_nsec / 1000000 - sStartTime - sPause;
#endif
}


float Time::GetSeconds()
{
	return (double)GetTick() / 1000.0;
}


uint64_t Time::WaitTick( uint64_t t, uint64_t last )
{
	uint64_t ticks = GetTick();
	if ( ( ticks - last ) < t ) {
		int64_t wait = t - (ticks - last) - 1;
		if ( wait >= 0 ) {
			Sleep( wait );
		}
	}
	return GetTick();
}


void Time::Sleep( uint64_t t )
{
	usleep( t * 1000 );
}
