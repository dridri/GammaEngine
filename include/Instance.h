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

#ifndef INSTANCE_H
#define INSTANCE_H

#include <stdint.h>
#include <typeinfo>
#include <string>

#undef CreateWindow

namespace GE {

class Window;
class Renderer;
class Renderer2D;
class DeferredRenderer;
class Vertex;
class Image;
class Object;

class Instance
{
public:
// 	Instance( const char* appName, uint32_t appVersion, bool easy_instance = true );
	Instance() : mDevId(0), mGpuCount(0), mGpus{0}, mDevices{0}, mQueues{0}, mFences{0}, mCpuRamCounter(0), mGpuRamCounter(0) {}
	virtual ~Instance(){}
	void Exit( int retcode = 0 );

	static Instance* Create( const char* appName, uint32_t appVersion, bool easy_instance = true, const std::string& backend_file = "" );
	virtual int EnumerateGpus() = 0;
	virtual Instance* CreateDevice( int devid, int queueCount = 1 ) = 0;
	virtual uint64_t ReferenceImage( Image* image ) = 0;
	virtual void UnreferenceImage( uint64_t ref ) = 0;
	virtual void UpdateImageData( Image* image, uint64_t ref ) = 0;

	Window* CreateWindow( const std::string& title, int width, int height, int flags = 0 );
	Renderer* CreateRenderer();
	Renderer2D* CreateRenderer2D( uint32_t width = 0, uint32_t height = 0 );
	DeferredRenderer* CreateDeferredRenderer( uint32_t width, uint32_t height );
	Object* CreateObject( Vertex* verts = nullptr, uint32_t nVerts = 0, uint32_t* indices = nullptr, uint32_t nIndices = 0 );
	Object* LoadObject( const std::string& filename );
// 	std::vector< Object* > LoadObjects( const std::string& filename );

	void* Memalign( uintptr_t size, uintptr_t align, bool clear_mem = true );
	void* Realloc( void* last, uintptr_t size, bool clear_mem = true );
	void* Malloc( uintptr_t size, bool clear_mem = true );
	void Free( void* data );

	void AffectRAM( int64_t sz );

	uint64_t gpu();
	uint64_t device();
	uint64_t queue();

	uint64_t cpuRamCounter();
	uint64_t gpuRamCounter();

	static std::string locale();
	static std::string username();
	static std::string useremail();
	static void setLocale( const std::string& s );
	static void setUserName( const std::string& s );
	static void setUserEmail( const std::string& s );

	static Instance* baseInstance();
	static uint64_t baseThread();
	static void* backend();

protected:
	static Instance* mBaseInstance;
	static uint64_t mBaseThread;
	static void* sBackend;
	static std::string sLocale;
	static std::string sUserName;
	static std::string sUserEmail;

	int mDevId;
	uint32_t mGpuCount;
	uint64_t mGpus[4];
	uint64_t mDevices[4];
	uint64_t mQueues[4];
	uint64_t mFences[4];

	uint64_t mCpuRamCounter;
	uint64_t mGpuRamCounter;
};

} // namespace GE

#if ( defined( GE_ANDROID ) )
#include <sstream>
namespace std {
template<typename T> static inline string to_string(const T& t) {
	ostringstream os;
	os << t;
	return os.str();
}
}
#endif

#if ( defined(GE_IOS) && !defined(GE_LIB) )
#define main _ge_main
#endif

#endif // INSTANCE_H
