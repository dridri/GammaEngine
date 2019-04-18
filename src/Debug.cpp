#include <time.h>

#include <mutex>
#include <map>

#define __DBG_CLASS
#include "Debug.h"
#include "include/Time.h"

namespace GE {

bool Debug::sEnabled = true;
bool Debug::sLineNumber = false;
bool Debug::sFilename = false;
bool Debug::mStoreLog = false;
std::string Debug::mLog = "";
std::mutex Debug::mLogMutex;

#ifdef GE_ANDROID
#include <android/log.h>
void Debug::log( const std::string& s_ ) {
	mLogMutex.lock();

	std::string s = s_;
	if ( s.length() >= 2 and s.at(s.length()-1) == '\n' and s.at(s.length()-2) == '\n' ) {
		s =  s.substr( 0, s.length() - 1 );
	}
	(void)__android_log_print( ANDROID_LOG_INFO, "GE", "%s", s.c_str() );
	if ( mStoreLog ) {
		mLog += s;
	}

	mLogMutex.unlock();
}
#else
void Debug::log( const std::string& s_ ) {
	mLogMutex.lock();

	std::string s = s_;
	if ( s.length() >= 2 and s.at(s.length()-1) == '\n' and s.at(s.length()-2) == '\n' ) {
		s =  s.substr( 0, s.length() - 1 );
	}
	std::cout << s << std::flush;
	if ( mStoreLog ) {
		mLog += s;
	}

	mLogMutex.unlock();
}
#endif


uint64_t Debug::GetTicks()
{
	return Time::GetTick();
}

} // namespace GE
