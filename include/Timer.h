#ifndef GE_TIMER_H
#define GE_TIMER_H

#include <stdint.h>

namespace GE
{

class Timer
{
public:
	Timer();
	void Start();
	void Pause();
	void Resume();
	void Stop();
	uint64_t ellapsed();
	bool isRunning();
	bool isPaused();
	void setShift( int64_t shift );

protected:
	bool mRunning;
	bool mPaused;
	uint64_t mStart;
	uint64_t mPauseTime;
	uint64_t mPause;
	int64_t mLastTime;
	int64_t mShift;
};
}

#endif // GE_TIMER_H
