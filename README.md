# GammaEngine
The Gamma Engine Library is a heavy game engine, providing every functions to create a complete game without any other external library. It will be multi-threaded by design, and only working with latest rendering technologies (except for GL ES 2.0 which is getting old, but many mobile devices are still using it).

Current status : far away from release

# Building
To compile GammaEngine, you will need the following dependancies :
 * CMake
 * libjpeg (currently version 9a)
 * libpng 1.6
 * libfreetype 2

mingw32-w64 is also required to build GE on win32 OS.

Pre-compiled binaries are also available here : http://ci.drich.fr/job/GammaEngine/

# Documentation
Full documentation is available here : http://doc.drich.fr

# Sample code
Here is the minimal source code to show a window using GammaEngine
```
#include <gammaengine/Instance.h>
#include <gammaengine/Window.h>
#include <gammaengine/Input.h>

using namespace GE;

int main( int ac, char** av )
{
	Instance* instance = Instance::Create( "GammaEngine application", 1 );
	Window* window = instance->CreateWindow( "Hello GammaEngine !", 1280, 720, Window::Resizable );

	while ( 1 ) {
		window->Clear( 0xFF202020 );
		window->SwapBuffers();
	}

	delete window;
	delete instance;
	return 0;
}
```
You can compile it using g++ -std=c++11 filename_you_choose.cpp -lgammaengine
