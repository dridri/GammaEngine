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

#ifndef TIME_H
#define TIME_H

#include <stdint.h>
#include <map>

namespace GE {

class Time {
public:
	Time();
	void setTimeParent( Time* t );

	static void setTime( int64_t t );
	static void GlobalSync();
	static double Delta();
	static uint64_t GetTick();
	static uint64_t GetTickMicros();
	static float GetSeconds();
	static uint64_t WaitTick( uint64_t t, uint64_t last );
	static void Sleep( uint64_t t );
	static void IncreasePause( int64_t t );

protected:
	double Sync();
	double SlowSync( double min );

private:
	static int64_t sStartTime;
// 	static int64_t sTime;
	static std::map< uint64_t, int64_t > sTime;
	static int64_t sPause;
// 	static double sDt;
	static std::map< uint64_t, double > sDt;

	double mDt;
	double mSlowDt;
	Time* mParent;
};

} // namespace GE

#endif // TIME_H
