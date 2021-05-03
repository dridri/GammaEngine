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

#ifndef DEBUG_H
#define DEBUG_H

#include <string.h>

#include <iostream>
#include <sstream>
#include <string>
#include <typeinfo>
#include <mutex>
#include <pthread.h>

#include "Instance.h"
#include <cxxabi.h>

namespace GE {

class Debug
{
public:
	Debug() {
	}
	~Debug() {
		if ( sEnabled ) {
			log( mSS.str() + "\n" );
		}
	}
	template<typename T> Debug& operator<<( const T& t ) {
		mSS << t;
		return *this;
	}

	static bool enabled() { return sEnabled; }
	static bool showFilename() { return sFilename; }
	static bool showLineNumber() { return sLineNumber; }
	static void setEnabled( bool en ) { sEnabled = en; }
	static void setShowLineNumber( bool en ) { sLineNumber = en; }
	static void setShowFilename( bool en ) { sFilename = en; }
	static void setStoreLog( bool st = false ) { mStoreLog = st; }

	static const std::string& DumpLog() { return mLog; }
	static uint64_t GetTicks();
	static uint64_t GetTicksMicros();

private:
	static void log( const std::string& s );
	static std::string mLog;
	std::stringstream mSS;
	static bool sEnabled;
	static bool sLineNumber;
	static bool sFilename;
	static bool mStoreLog;
	static std::mutex mLogMutex;
};


#ifndef __DBG_CLASS
static inline std::string className(const std::string& prettyFunction)
{
	size_t parenthesis = prettyFunction.find("(");
	size_t colons = prettyFunction.substr( 0, parenthesis ).rfind("::");
	if (colons == std::string::npos)
		return "<none>";
	size_t begin = prettyFunction.substr(0,colons).rfind(" ") + 1;
	size_t end = colons - begin;
	return "\x1B[32m" + prettyFunction.substr(begin,end) + "\x1B[0m";
}


#pragma GCC system_header // HACK Disable unused-function warnings
static std::string self_thread() {
#ifdef GE_ANDROID
	return "";
#endif
	std::stringstream ret;
	char name[128] = "";
	memset( name, 0, sizeof(name) );
#ifdef pthread_getname_np
	pthread_getname_np( pthread_self(), name, sizeof(name)-1 );
#endif
	ret << "\x1B[33m" << "[" << name << "] " << "\x1B[0m";
	return ret.str();
}

#define __CLASS_NAME__ className(__PRETTY_FUNCTION__)
#define __FUNCTION_NAME__ ( std::string("\x1B[94m") + __FUNCTION__ + "\x1B[0m" )

#pragma GCC system_header // HACK Disable unused-function warnings
static void fDebug_base( std::stringstream& out, const char* end, bool f ) {
	out << " " << end;
}

template<typename Arg1, typename... Args> static void fDebug_base( std::stringstream& out, const char* end, bool first, const Arg1& arg1, const Args&... args ) {
	char* type = abi::__cxa_demangle(typeid(arg1).name(), nullptr, nullptr, nullptr);
	char cap = 0;
	std::string color = "\x1B[0m";
// 	out << "[TYPE:" << type << "]";
	if ( strstr( type, "char" ) || strstr( type, "string" ) ) {
		if ( strstr( type, "*" ) || ( strstr( type, "[" ) && strstr( type, "]" ) ) || strstr( type, "string" ) ) {
			cap = '\"';
			color = "\x1B[31m";
		} else {
			cap = '\'';
			color = "\x1B[31m";
		}
	}

	std::stringstream arg_ss;
	if ( strstr( type, "bool" ) ) {
		bool valid = false;
		arg_ss << arg1;
		if ( arg_ss.str()[0] == '0' ) {
			color = "\x1B[91m";
			arg_ss.str("");
			arg_ss << "false";
		} else {
			color = "\x1B[92m";
			arg_ss.str("");
			arg_ss << "true";
		}
	} else {
		arg_ss << arg1;
		if ( ( arg_ss.str()[0] >= '0' && arg_ss.str()[0] <= '9' ) || ( ( arg_ss.str()[0] == '-' || arg_ss.str()[0] == '+' ) && arg_ss.str()[1] >= '0' && arg_ss.str()[1] <= '9' ) ) {
			color = "\x1B[36m";
		}
	}
	free(type);

	if (!first ) {
		out << ", ";
	}
	std::stringstream ss;
	ss << color;
	if ( cap ) ss << cap;
	ss << arg_ss.str();
	if ( cap ) ss << cap;
	ss << "\x1B[0m";
	out << ss.str();
	fDebug_base( out, end, false, args... );
}

#pragma GCC system_header // HACK Disable unused-function warnings
template<typename... Args> static std::string fDebug_top( const Args&... args ) {
	if ( not Debug::enabled() ) {
		return "";
	}
	std::stringstream out;
	if ( sizeof...(args) == 0 ) {
		out << ")";
	} else {
		out << " ";
		fDebug_base( out, ")", true, args... );
	}
	return out.str();
}

#define gDebug() Debug() << self_thread() << (Debug::showFilename() ? (std::string(__FILE__)+":") : "") << (Debug::showLineNumber() ? (std::to_string(__LINE__)+": ") : "") << __CLASS_NAME__ << "::" << __FUNCTION_NAME__ << "() "
#define fDebug( ... ) { Debug dbg; dbg << self_thread() << (Debug::showFilename() ? (std::string(__FILE__)+":") : "") << (Debug::showLineNumber() ? (std::to_string(__LINE__)+": ") : ""); dbg << __CLASS_NAME__ << "::" << __FUNCTION_NAME__ << "("; dbg << fDebug_top( __VA_ARGS__ ); }


#ifndef fDebug
extern void fDebug(); // KDevelop hack
#endif fDebug

#endif // __DBG_CLASS


} // namespace GE

#endif // DEBUG_H
