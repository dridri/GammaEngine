#include <time.h>

#include <mutex>
#include <map>

#define __DBG_CLASS
#include "Debug.h"
#include "include/Time.h"

namespace GE {

bool Debug::mStoreLog = false;
std::string Debug::mLog = std::string();
std::mutex Debug::mLogMutex;

#ifdef GE_ANDROID
#include <android/log.h>
static std::string str;
void Debug::log( const std::string& s ) {
	str = str + s;
// 	__android_log_print( ANDROID_LOG_INFO, "GE", "STR : '%s'", str.c_str() );

	int pos = -1;
	if ( str.find( "\n" ) > 0 ) {
		while ( ( pos = str.find( "\n" ) ) > 0 ) {
			std::string sub = str.substr( 0, pos );
			(void)__android_log_print( ANDROID_LOG_INFO, "GE", "%s", sub.c_str() );
			if ( mStoreLog ) {
				mLogMutex.lock();
				mLog += sub + "\n";
				mLogMutex.unlock();
			}
			str = str.substr( pos + 1 );
		}
	}

}
#else
void Debug::log( const std::string& s ) {
	std::cout << s << std::flush;
	if ( mStoreLog ) {
		mLogMutex.lock();
		mLog += s;
		mLogMutex.unlock();
	}
}
#endif

//static std::mutex mMutex;
//static std::map< pthread_t, std::ostringstream* > mOutput;
//static std::map< pthread_t, uint64_t > mTicks;

Debug::Debug()
{
}


Debug::~Debug()
{
}


uint64_t Debug::GetTicks()
{
	return Time::GetTick();
}

} // namespace GE
