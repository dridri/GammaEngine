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
 
 
#include <stdlib.h>
#include "Instance.h"
#include "AudioOutput.h"
#include "BaseWindow.h"
#include "Debug.h"

extern "C" int XInitThreads(void);

#ifndef ALIGN
	#define ALIGN(x, align) (((x)+((align)-1))&~((align)-1))
#endif

#ifdef GE_WIN32
	#include <windows.h>
	#undef CreateWindow
	void* LoadLib( const std::string& file ) {
		return LoadLibrary( file.c_str() );
	}
	void* SymLib( void* lib, const char* name ) {
		return (void*)GetProcAddress( (HMODULE)lib, name );
	}
	const std::string LibError() {
		DWORD errorMessageID = ::GetLastError();
		if ( errorMessageID == 0 ) {
		    return "No error message has been recorded";
		}
		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
		std::string message(messageBuffer, size);
		LocalFree(messageBuffer);
		return message;
	}
#else
	#include <dlfcn.h>
	void* LoadLib( const std::string& file ) {
		std::cout << file << "\n";
		return dlopen( file.c_str(), RTLD_LAZY );
	}
	void* SymLib( void* lib, const char* name ) {
		return dlsym( lib, name );
	}
	const std::string LibError() {
		return dlerror();
	}
#endif

using namespace GE;


Instance* Instance::mBaseInstance = nullptr;
uint64_t Instance::mBaseThread = 0;

std::string Instance::sLocale = "";
std::string Instance::sUserName = "";
std::string Instance::sUserEmail = "";

Instance* Instance::baseInstance()
{
	return mBaseInstance;
}


uint64_t Instance::baseThread()
{
	return mBaseThread;
}


void* Instance::backend()
{
	return mBackend;
}

#ifdef GE_STATIC_BACKEND

extern "C" Instance* CreateInstance( void*, const char*, uint32_t );
extern "C" Window* CreateWindow( Instance*, const std::string&, int, int, int );
extern "C" Renderer* CreateRenderer( Instance* );
extern "C" Renderer2D* CreateRenderer2D( Instance*, uint32_t, uint32_t );
extern "C" DeferredRenderer* CreateDeferredRenderer( Instance*, uint32_t, uint32_t );
extern "C" RenderBuffer* CreateRenderBuffer( Instance*, uint32_t, uint32_t );
extern "C" Object* CreateObject( VertexBase*, uint32_t, uint32_t*, uint32_t );
extern "C" Object* LoadObject( const std::string&, Instance* );

Instance* Instance::Create( const char* appName, uint32_t appVersion, bool easy_instance, const std::string& backend_file )
{
	if ( easy_instance ) {
		if ( !mBaseInstance ) {
			mBaseInstance = CreateInstance( nullptr, appName, appVersion );
			mBaseThread = (uint64_t)pthread_self();
		}
		Instance* ret = mBaseInstance->CreateDevice( 0, 1 );
		if ( !mBaseInstance || mBaseInstance->device() == 0 ) {
			mBaseInstance = ret;
		}
		return ret;
	}
	return CreateInstance( nullptr, appName, appVersion );
}

Window* Instance::CreateWindow( const std::string& title, int width, int height, int flags )
{
	return ::CreateWindow( this, title, width, height, flags );
}


Renderer* Instance::CreateRenderer()
{
	return ::CreateRenderer( this );
}


Renderer2D* Instance::CreateRenderer2D( uint32_t width, uint32_t height )
{
	return ::CreateRenderer2D( this, width, height );
}


DeferredRenderer* Instance::CreateDeferredRenderer( uint32_t width, uint32_t height )
{
	return ::CreateDeferredRenderer( this, width, height );
}


RenderBuffer* Instance::CreateRenderBuffer( uint32_t width, uint32_t height )
{
	return ::CreateRenderBuffer( this, width, height );
}


Object* Instance::CreateObject( VertexBase* verts, uint32_t nVert, uint32_t* indices, uint32_t nIndices )
{
	return ::CreateObject( verts, nVert, indices, nIndices );
}


Object* Instance::LoadObject( const std::string& filename )
{
	return ::LoadObject( filename, this );
}

#else // GE_STATIC_BACKEND

Instance* Instance::Create( const char* appName, uint32_t appVersion, bool easy_instance, const std::string& backend_file )
{
	void* local_backend = nullptr;

#if ( defined( GE_LINUX ) && !defined( GE_EGL ) )
	XInitThreads();
#endif

#ifdef GE_WIN32
	std::string lib_suffix = ".dll";
#else
	std::string lib_suffix = ".so";
#endif
#if ( defined( GE_ANDROID ) || defined( GE_IOS ) )
	std::string backend_lib = "opengles20";
#else
// 		std::string backend_lib = "vulkan";
	std::string backend_lib = "opengl43";
#endif
	std::string prefixes[10] = {
		"backend_",
		"backends/",
		"gammaengine/backend_",
		"gammaengine/backends/",
		"/usr/local/lib/backend_",
		"/usr/local/lib/gammaengine/backend_",
		"/usr/local/lib/gammaengine/backends/",
		"/usr/lib/backend_",
		"/usr/lib/gammaengine/backend_",
		"/usr/lib/gammaengine/backends/",
	};
	if ( backend_file != "" ) {
		backend_lib = backend_file;
	}
	int i = 0;
	for ( i = 0; i < 10 && !local_backend; i++ ) {
		local_backend = LoadLib( prefixes[i] + backend_lib + lib_suffix );
		if ( local_backend == nullptr ) {
#ifdef GE_WIN32
			gDebug() << "Backend ( " << backend_lib << " ) loading error : " << ( prefixes[i] + backend_lib + lib_suffix ) << " : " << LibError() << "\n";
#else
			gDebug() << "Backend ( " << backend_lib << " ) loading error : " << LibError() << "\n";
#endif
		}
	}
	if ( local_backend == nullptr ) {
		exit(0);
	} else {
		gDebug() << "Backend file " << prefixes[--i] << backend_lib << lib_suffix << " loaded !\n";
	}

	typedef Instance* (*f_type)( void*, const char*, uint32_t );
	f_type fCreateInstance = (f_type)SymLib( local_backend, "CreateInstance" );
	if ( easy_instance ) {
		if ( !mBaseInstance ) {
			mBaseInstance = fCreateInstance( local_backend, appName, appVersion );
			mBaseThread = pthread_self();
		}
		Instance* ret = mBaseInstance->CreateDevice( 0, 1 );
		if ( !mBaseInstance || mBaseInstance->device() == 0 ) {
			mBaseInstance = ret;
		}
		return ret;
	}
	return fCreateInstance( local_backend, appName, appVersion );
}


Window* Instance::CreateWindow( const std::string& title, int width, int height, int flags )
{
	typedef Window* (*f_type)( Instance*, const std::string&, int, int, int );
	f_type fCreateWindow = (f_type)SymLib( backend(), "CreateWindow" );
	return fCreateWindow( this, title, width, height, flags );
}


Renderer* Instance::CreateRenderer()
{
	typedef Renderer* (*f_type)( Instance* );
	f_type fCreateRenderer = (f_type)SymLib( backend(), "CreateRenderer" );
	return fCreateRenderer( this );
}


Renderer2D* Instance::CreateRenderer2D( uint32_t width, uint32_t height )
{
	typedef Renderer2D* (*f_type)( Instance*, uint32_t, uint32_t );
	f_type fCreateRenderer2D = (f_type)SymLib( backend(), "CreateRenderer2D" );
	return fCreateRenderer2D( this, width, height );
}


DeferredRenderer* Instance::CreateDeferredRenderer( uint32_t width, uint32_t height )
{
	typedef DeferredRenderer* (*f_type)( Instance*, uint32_t, uint32_t );
	f_type fCreateDeferredRenderer = (f_type)SymLib( backend(), "CreateDeferredRenderer" );
	return fCreateDeferredRenderer( this, width, height );
}


RenderBuffer* Instance::CreateRenderBuffer( uint32_t width, uint32_t height )
{
	typedef RenderBuffer* (*f_type)( Instance*, uint32_t, uint32_t );
	f_type fCreateRenderBuffer = (f_type)SymLib( backend(), "CreateRenderBuffer" );
	return fCreateRenderBuffer( this, width, height );
}


Object* Instance::CreateObject( VertexBase* verts, uint32_t nVert, uint32_t* indices, uint32_t nIndices )
{
	typedef Object* (*f_type)( VertexBase*, uint32_t, uint32_t*, uint32_t );
	f_type fCreateObject = (f_type)SymLib( backend(), "CreateObject" );
	return fCreateObject( verts, nVert, indices, nIndices );
}


Object* Instance::LoadObject( const std::string& filename )
{
	typedef Object* (*f_type)( const std::string&, Instance* );
	f_type fLoadObject = (f_type)SymLib( backend(), "LoadObject" );
	return fLoadObject( filename, this );
}
#endif // GE_STATIC_BACKEND


void Instance::Exit( int retcode )
{
#ifdef GE_ANDROID
	BaseWindow::AndroidExit( retcode );
#else
	exit( retcode );
#endif
}


static uintptr_t _ge_AllocMemBlock( uintptr_t size )
{
	return (uintptr_t)malloc( size );
}


void* Instance::Memalign( uintptr_t size, uintptr_t align, bool clear_mem )
{
	uintptr_t block_sz = sizeof(uintptr_t) * 2;
	size_t fullSize = size + ( align > block_sz ? align : block_sz ) + align;
	uintptr_t addr = _ge_AllocMemBlock( fullSize );
	uintptr_t* var = (uintptr_t*)( ALIGN( addr + ( align > block_sz ? align : block_sz ), align ) - block_sz );
	var[0] = addr;
// 	var[1] = size;
	var[1] = fullSize;
	if ( clear_mem ) {
		memset( (void*)(uintptr_t)&var[2], 0x0, size );
	}
// 	mCpuRamCounter += size;
	mCpuRamCounter += fullSize;
	return (void*)(uintptr_t)&var[2];
}


void* Instance::Malloc( uintptr_t size, bool clear_mem )
{
	if ( size <= 0 ) {
		return NULL;
	}
	return Memalign( size, 16, clear_mem );
}


void Instance::Free( void* data )
{
	if ( data != NULL && data != (void*)0xDEADBEEF && data != (void*)0xBAADF00D ) {
		uintptr_t* var = (uintptr_t*)data;
		uintptr_t addr = var[-2];
		size_t size = var[-1];
		free( (void*)addr );
		mCpuRamCounter -= size;
	}
}


void* Instance::Realloc( void* last, uintptr_t size, bool clear_mem )
{
	if ( size <= 0 ) {
		Free( last );
		return NULL;
	}
	if ( last == NULL || last == (void*)0xDEADBEEF || last == (void*)0xBAADF00D ) {
		return Malloc( size, clear_mem );
	}
	uintptr_t last_size = ((uintptr_t*)last)[-1];
	
	const int align = 16;
	uintptr_t block_sz = sizeof(uintptr_t) * 2;
	size_t fullSize = size + ( align > block_sz ? align : block_sz ) + align;
	uintptr_t addr = _ge_AllocMemBlock( fullSize );
	uintptr_t* var = (uintptr_t*)( ALIGN( addr + ( align > block_sz ? align : block_sz ), align ) - block_sz );
// 	size_t fullSize = size + sizeof(uintptr_t) * 2 + 16;
// 	uintptr_t addr = _ge_AllocMemBlock( fullSize );
// 	uintptr_t* var = (uintptr_t*)(ALIGN(addr + sizeof(uintptr_t) * 2, 16 ) - sizeof(uintptr_t) * 2 );
	var[0] = addr;
	var[1] = size;
	void* new_ptr = (void*)&var[2];
	if ( clear_mem ) {
		memset( (void*)(uintptr_t)&var[2], 0x0, size );
	}

	uintptr_t sz_copy = last_size < size ? last_size : size;
	memcpy(new_ptr, last, sz_copy);

	var = (uintptr_t*)last;
	size_t lastSize = var[-1];
	free((void*)var[-2]);

	mCpuRamCounter -= lastSize;
	mCpuRamCounter += size;

	return new_ptr;
}


void Instance::AffectRAM( int64_t sz )
{
	mCpuRamCounter += sz;
}


uint64_t Instance::cpuRamCounter()
{
	return mCpuRamCounter;
}


uint64_t Instance::gpuRamCounter()
{
	return mGpuRamCounter;
}


uint64_t Instance::gpu()
{
	return mGpus[ mDevId ];
}


uint64_t Instance::device()
{
	return mDevices[mDevId];
}


uint64_t Instance::queue()
{
	return mQueues[mDevId];
}


std::string Instance::locale()
{
	return sLocale;
}


std::string Instance::username()
{
	return sUserName;
}


std::string Instance::useremail()
{
	return sUserEmail;
}


void Instance::setLocale( const std::string& s )
{
	sLocale = s;
}


void Instance::setUserName( const std::string& s )
{
	sUserName = s;
}


void Instance::setUserEmail( const std::string& s )
{
	sUserEmail = s;
}


// } // namespace GE
