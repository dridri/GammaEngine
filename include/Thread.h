/*
 * The GammaEngine Library 2.0 is a multiplatform Vulkan-based game engine
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

#ifndef GE_THREAD_H
#define GE_THREAD_H

#include <thread>
#include <pthread.h>

namespace GE
{

class Window;

class Thread
{
public:
	Thread( Window* shared_graphics_window = nullptr );
	virtual ~Thread();

	void Start();
	void Pause();
	void Stop();
	void Join();
	bool running();

protected:
	virtual bool run() = 0;

private:
	static void sThreadEntry( Thread* thiz );
	void mThreadEntry();
	Window* mSharedWindow;
	uint64_t mSharedContext;
	bool mRunning;
	bool mIsRunning;
	bool mFinished;
// 	std::thread* mThread;
	pthread_t mThread;
};

}

#endif // GE_THREAD_H
