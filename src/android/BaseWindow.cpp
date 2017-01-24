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

#define BASEWINDOW_CPP

#include <stdlib.h>
#include <string.h>
#include "android/BaseWindow.h"
#include "Window.h"
#include "Instance.h"
#include "Time.h"
#include "Input.h"
#include "Debug.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "GE", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "GE", __VA_ARGS__))

int main( int, char** );

extern "C" void android_main( struct android_app* state )
{
	LOGW("\n\n\n\n");
	LOGW("===================== GammaEngine for Android =====================\n");

	GE::BaseWindow::AndroidInit( state );
}

namespace GE {

Engine* BaseWindow::mEngine = nullptr;
struct android_app* BaseWindow::mState = nullptr;

bool BaseWindow::mKeys[512];
Vector2i BaseWindow::mCursor;
Vector2i BaseWindow::mCursorWarp;
bool BaseWindow::mAdsVisible = false;
uint64_t BaseWindow::mPauseTime = 0;
bool GE::BaseWindow::mExiting = false;
bool GE::BaseWindow::mExitApp = false;


void BaseWindow::AndroidInit( struct android_app* state )
{
	mState = state;
	mEngine = new Engine;
	memset( mEngine, 0, sizeof( Engine ) );
	state->userData = mEngine;
	state->onAppCmd = &BaseWindow::engine_handle_cmd;
	state->onInputEvent = &BaseWindow::engine_handle_input;
	mEngine->app = state;
	mEngine->aSurface = mEngine->app->window;
	mEngine->gotFocus = true;
	mEngine->cursorId = -1;

	while ( !state->window ) {
		PollEvents();
	}

	main( 0, nullptr );
}


void GE::BaseWindow::AndroidExit( int retcode )
{
	fDebug( retcode );
	if ( mExitApp ) {
		return; // Already exiting
	}
	mExiting = true;
	ANativeActivity_finish( mEngine->app->activity );
	while ( mExiting ) {
		Time::Sleep( 10 );
		PollEvents();
	}
}


BaseWindow::BaseWindow( Instance* instance, const std::string& title, int width, int height, uint32_t _flags )
	: mInstance( instance )
	, mWidth( width )
	, mHeight( height )
	, mHasResized( false )
	, mWindow( 0 )
{
	memset( mKeys, 0, sizeof( mKeys ) );
	mCursor = mCursorWarp = Vector2i( 0, 0 );
	Window::Flags flags = static_cast<Window::Flags>( _flags );
/*
	mEngine->aflags = AWINDOW_FLAG_KEEP_SCREEN_ON;
	if ( flags & Window::Fullscreen ) {
		mEngine->aflags |= AWINDOW_FLAG_FULLSCREEN;
// 		mEngine->aSurface = 0;
	}

	ANativeActivity_setWindowFlags( mState->activity, mEngine->aflags, 0 );
*/
	while ( mEngine->hasSurface && !mEngine->aSurface ) {
		Time::Sleep( 10 );
	}
	if ( !mEngine->aSurface ) {
		mEngine->aSurface = mEngine->app->window;
	}

	const EGLint attribs[] = {
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_BLUE_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_RED_SIZE, 8,
			EGL_ALPHA_SIZE, 8,
			EGL_DEPTH_SIZE, 16,
			EGL_NONE
	};
	const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLint w, h, dummy, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
	eglInitialize( display, 0, 0 );
	eglChooseConfig( display, attribs, &config, 1, &numConfigs );
	eglGetConfigAttrib( display, config, EGL_NATIVE_VISUAL_ID, &format );

	LOGI( "Surfaces : %p  %p", mEngine->aSurface, mEngine->app->window );

	int base_width = 0;
	int base_height = 0;
	if ( mEngine->app && mEngine->aSurface ) {
		base_width = ANativeWindow_getWidth( mEngine->aSurface );
		base_height = ANativeWindow_getHeight( mEngine->aSurface );
	}
	LOGI( "base size: %d x %d", base_width, base_height );

	mWidth = width < 0 ? base_width : width;
	mHeight = height < 0 ? base_height : height;
	LOGI( "new size: %d x %d", mWidth, mHeight );

	ANativeWindow_setBuffersGeometry( mEngine->aSurface, mWidth, mHeight, WINDOW_FORMAT_RGBA_8888/*format*/ );
	surface = eglCreateWindowSurface( display, config, mEngine->aSurface, nullptr );

	if ( mEngine->context == EGL_NO_CONTEXT ) {
		context = eglCreateContext( display, config, nullptr, context_attribs );
	} else {
		context = mEngine->context;
	}
	if ( eglMakeCurrent( display, surface, surface, context ) == EGL_FALSE ) {
		LOGW( "Unable to eglMakeCurrent" );
	}

	eglQuerySurface( display, surface, EGL_WIDTH, &w );
	eglQuerySurface( display, surface, EGL_HEIGHT, &h );

	mWidth = w;
	mHeight = h;
	LOGI("final size: %d x %d", w, h);

	mEngine->format = format;
	mEngine->config = config;
	mEngine->display = display;
	mEngine->context = context;
	mEngine->surface = surface;
	mEngine->width = w;
	mEngine->height = h;
	mEngine->mainWindowCreated = true;
}


BaseWindow::~BaseWindow()
{
}


uint32_t BaseWindow::width()
{
	return mWidth;
}


uint32_t BaseWindow::height()
{
	return mHeight;
}


Vector2i& BaseWindow::cursor()
{
	return mCursor;
}


Vector2i& BaseWindow::cursorWarp()
{
	return mCursorWarp;
}


float BaseWindow::fps() const
{
	return mFps;
}


void BaseWindow::SwapBuffersBase()
{
	mFpsImages++;
	if ( Time::GetTick() - mFpsTimer > 500 ) {
		mFps = mFpsImages * 1000.0f / ( Time::GetTick() - mFpsTimer );
		mFpsTimer = Time::GetTick();
		mFpsImages = 0;
	}

	mCursorWarp.x = 0;
	mCursorWarp.y = 0;

	PollEvents();
	while ( mEngine->surface == EGL_NO_SURFACE and not mExitApp ) {
		Time::Sleep(100);
		PollEvents();
	}

	mWidth = mEngine->width;
	mHeight = mEngine->height;
	if ( mEngine->surface != EGL_NO_SURFACE ) {
		eglSwapBuffers( mEngine->display, mEngine->surface );
	}

	if ( mEngine->justLostFocus ) {
		mEngine->justLostFocus = false;
	}
}


void BaseWindow::engine_term_display( Engine* engine )
{
	if ( engine->display != EGL_NO_DISPLAY ) {
		eglMakeCurrent( engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
	}
	if ( engine->context != EGL_NO_CONTEXT ) {
		eglDestroyContext( engine->display, engine->context );
	}
	if ( engine->surface != EGL_NO_SURFACE ) {
		eglDestroySurface( engine->display, engine->surface );
	}
	if ( engine->aSurface ) {
		ANativeWindow_release( engine->aSurface );
	}
	if ( engine->display != EGL_NO_DISPLAY ) {
		eglTerminate(engine->display);
	}
	engine->display = EGL_NO_DISPLAY;
	engine->context = EGL_NO_CONTEXT;
	engine->surface = EGL_NO_SURFACE;
	engine->aSurface = 0;
}


void BaseWindow::engine_handle_cmd( struct android_app* app, int32_t cmd )
{
	int w, h;
	switch ( cmd ) {
		case APP_CMD_SAVE_STATE:
			gDebug() << "APP_CMD_SAVE_STATE\n";
			break;
		case APP_CMD_INIT_WINDOW:
			gDebug() << "APP_CMD_INIT_WINDOW\n";
			if ( mEngine->app->window != nullptr && mEngine->mainWindowCreated ) {
				while ( mEngine->hasSurface && !mEngine->aSurface ) {
					Time::Sleep(10);
				}

				eglMakeCurrent( mEngine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
				ANativeActivity_setWindowFlags( mState->activity, mEngine->aflags, 0 );
				eglDestroySurface( mEngine->display, mEngine->surface );
				ANativeWindow_setBuffersGeometry( mEngine->aSurface, 0, 0, /*mEngine->format*/WINDOW_FORMAT_RGBA_8888 );
				mEngine->display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
				mEngine->surface = eglCreateWindowSurface( mEngine->display, mEngine->config, mEngine->aSurface, nullptr );
				eglMakeCurrent( mEngine->display, mEngine->surface, mEngine->surface, mEngine->context );
				eglQuerySurface( mEngine->display, mEngine->surface, EGL_WIDTH, &w );
				eglQuerySurface( mEngine->display, mEngine->surface, EGL_HEIGHT, &h );
				mEngine->width = w;
				mEngine->height = h;
			}
			break;
		case APP_CMD_TERM_WINDOW:
			gDebug() << "APP_CMD_TERM_WINDOW\n";
			eglMakeCurrent( mEngine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
			if ( mEngine->surface != EGL_NO_SURFACE ) {
				eglDestroySurface(mEngine->display, mEngine->surface);
				mEngine->surface = EGL_NO_SURFACE;
			}

			if ( mEngine->hasSurface && mEngine->aSurface ) {
				ANativeWindow_release( mEngine->aSurface );
				mEngine->aSurface = 0;
			}

			break;
		case APP_CMD_GAINED_FOCUS:
			gDebug() << "APP_CMD_GAINED_FOCUS\n";
// 			ge_ResumeAllThreads(); // TODO
			Time::IncreasePause( Time::GetTick() - mPauseTime );
			mEngine->gotFocus = true;
			break;
		case APP_CMD_LOST_FOCUS:
			gDebug() << "APP_CMD_LOST_FOCUS\n";
			for ( uint32_t i = 0; i < 16; i++ ) {
				if ( mEngine->touches[i].used ) {
					mEngine->touches[i].used = false;
					mEngine->touches[i].id = AMOTION_EVENT_ACTION_UP;
				}
			}
			mKeys[ Input::LBUTTON ] = false;
			mEngine->cursorId = -1;
// 			ge_PauseAllThreads(); // TODO
			mPauseTime = Time::GetTick();
			mEngine->gotFocus = false;
			mEngine->justLostFocus = true;
			break;
	}
}


int32_t BaseWindow::engine_handle_input( struct android_app* app, AInputEvent* event )
{
	Engine* engine = mEngine;
	ATouch* touches = engine->touches;

    if ( AInputEvent_getType( event ) == AINPUT_EVENT_TYPE_KEY ) {
		int action = AKeyEvent_getAction( event );
		if ( AKeyEvent_getKeyCode( event ) == AKEYCODE_BACK ) {
			if ( action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_POINTER_DOWN ) {
				gDebug() << "Back button pressed\n";
				mKeys[ Input::BACK ] = true;
			}
			if ( action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_POINTER_UP ) {
				gDebug() << "Back button released\n";
				mKeys[ Input::BACK ] = false;
			}
			return 1;
		}
	}

    if ( AInputEvent_getType( event ) == AINPUT_EVENT_TYPE_MOTION ) {
		int n = AMotionEvent_getPointerCount( event );
		int action = AMotionEvent_getAction( event ) & AMOTION_EVENT_ACTION_MASK;
		int pointer_index = ( AMotionEvent_getAction( event ) & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK ) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

		for ( int j = 0; j < n; j++ ) {
			ATouch* touch = nullptr;
			for ( uint32_t i = 0; i < 16; i++ ) {
				/*
				if ( touches[i].used && ( ( touches[i].action == AMOTION_EVENT_ACTION_UP ) || ( touches[i].action == AMOTION_EVENT_ACTION_POINTER_UP ) ) ) {
					touches[i].used = false;
					touches[i].id = -1;
					if ( touches[i].id == engine->cursorId ) {
						engine->cursorId = -1;
					}
				}
				*/
				if ( touches[i].used && touches[i].id == AMotionEvent_getPointerId( event, j ) ) {
					touch = &touches[i];
					break;
				}
			}
			if ( !touch && ( action != AMOTION_EVENT_ACTION_UP ) && ( action != AMOTION_EVENT_ACTION_POINTER_UP ) ) {
				for ( uint32_t i = 0; i < 16; i++ ) {
					if ( touches[i].used == false ) {
						touch = &touches[i];
						touch->used = true;
						touch->id = AMotionEvent_getPointerId( event, j );
						break;
					}
				}
			}
			if( touch ) {
				if ( j == pointer_index ) {
					touch->action = action;
				}
				touch->x = AMotionEvent_getX( event, j ) * engine->width / ANativeWindow_getWidth( engine->app->window );
				touch->y = AMotionEvent_getY( event, j ) * engine->height / ANativeWindow_getHeight( engine->app->window );
				touch->force = AMotionEvent_getPressure( event, j );
				if ( engine->cursorId < 0 || ( touch->id <= engine->cursorId && touch->used ) ) {
					if ( action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_POINTER_DOWN ) {
						mKeys[ Input::LBUTTON ] = true;
					}
					if ( action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_POINTER_UP ) {
						mKeys[ Input::LBUTTON ] = false;
					}
// 					engine->last_cPress = cPress;
// 					cPress = v_keys[ Input::LBUTTON ];
					engine->cursorId = touch->id;
					Vector2i lastCursor = mCursor;
					mCursor.x = (int)touch->x;
					mCursor.y = (int)touch->y - engine->ofsy * engine->height / ANativeWindow_getHeight( engine->app->window );
// 					if ( !engine->last_cPress ) {
// 						mouse_last_x = libge_context->mouse_x;
// 						mouse_last_y = libge_context->mouse_y;
// 					}
					mCursorWarp = mCursor - lastCursor;
				}
			}
		}
        return 1;
    }
	return 0;
}


void BaseWindow::PollEvents()
{
	int ident;
	int events;
	struct android_poll_source* source;
	uint64_t pause_start = Time::GetTick();

	Engine* engine = mEngine;
	ATouch* touches = engine->touches;
	for ( uint32_t i = 0; i < 16; i++ ) {
		if ( touches[i].used && ( ( touches[i].action == AMOTION_EVENT_ACTION_UP ) || ( touches[i].action == AMOTION_EVENT_ACTION_POINTER_UP ) ) ) {
			touches[i].used = false;
			touches[i].id = -1;
			if ( touches[i].id == engine->cursorId ) {
				engine->cursorId = -1;
			}
		}
	}

	do {
		while ( ( ident = ALooper_pollAll( 0, nullptr, &events, (void**)&source) ) >= 0 ) {
			if (source != nullptr) {
				source->process( mState, source );
			}
			if ( mState->destroyRequested != 0 ) {
				gDebug() << "destroy requested\n";
				engine_term_display( mEngine );
				gDebug() << "display destroyed\n";
				mExiting = false;
				mExitApp = true;
				ANativeActivity_finish( mEngine->app->activity );
// 				exit( 0 );
				return;
			}
		}
		if ( mEngine->gotFocus == false and mEngine->justLostFocus == false ) {
			Time::Sleep( 50 );
		}
	} while ( mEngine->gotFocus == false and mEngine->justLostFocus == false );
}


void BaseWindow::ShowInterstitialAd()
{
	fDebug();
	mAdsVisible = true;
	while ( mAdsVisible ) {
		PollEvents();
		Time::Sleep( 50 );
	}
	mCursorWarp = Vector2i();
}


extern "C" JNIEXPORT void JNICALL Java_com_drich_libge_LibGE_setSurface( JNIEnv* env, jobject obj, jobject surface, jint ofsx, jint ofsy )
{
	fDebug( env, obj, surface, ofsx, ofsy );
	BaseWindow::androidEngine()->aJavaSurface = surface;
	BaseWindow::androidEngine()->ofsx = ofsx;
	BaseWindow::androidEngine()->ofsy = ofsy;
	BaseWindow::androidEngine()->aSurface = ANativeWindow_fromSurface( env, surface );
	gDebug() << " ===> " << BaseWindow::androidEngine()->aSurface << "\n";
}


extern "C" JNIEXPORT void JNICALL Java_com_drich_libge_LibGE_setHasSurface( JNIEnv* env, jobject obj, jint _hasSurface )
{
	fDebug( env, obj, _hasSurface );

	while ( !BaseWindow::androidEngine() ) {
		Time::Sleep( 1 );
	}
	BaseWindow::androidEngine()->hasSurface = _hasSurface;
}


static std::string ge_jstring_to_string( JNIEnv* env, jstring string )
{
	const char* s = env->GetStringUTFChars(string, nullptr );
	std::string ret = std::string( s );
	env->ReleaseStringUTFChars( string, s );
	return ret;
}


extern "C" JNIEXPORT void JNICALL Java_com_drich_libge_LibGE_AndroidInit( JNIEnv* env, jobject obj, jstring locale, jstring username, jstring email )
{
	fDebug( env, obj, ge_jstring_to_string( env, locale ), ge_jstring_to_string( env, username ), ge_jstring_to_string( env, email ) );

	Instance::setLocale( ge_jstring_to_string( env, locale ) );
	Instance::setUserName( ge_jstring_to_string( env, username ) );
	Instance::setUserEmail( ge_jstring_to_string( env, email ) );
}


extern "C" JNIEXPORT void JNICALL Java_com_drich_libge_LibGE_setAdsVisible( JNIEnv* env, jobject obj, jboolean en )
{
	fDebug( env, obj, en );
	BaseWindow::mAdsVisible = en;
}


extern "C" JNIEXPORT jboolean JNICALL Java_com_drich_libge_LibGE_adsVisible( JNIEnv* env, jobject obj )
{
// 	fDebug( env, obj );
	return BaseWindow::mAdsVisible;
}


extern "C" JNIEXPORT jboolean JNICALL Java_com_drich_libge_LibGE_exiting( JNIEnv* env, jobject obj )
{
// 	fDebug( env, obj );
// 	gDebug() << "  => " << BaseWindow::mExitApp << "\n";
	return BaseWindow::mExitApp;
}


extern "C" JNIEXPORT void JNICALL Java_com_drich_libge_LibGE_backButtonPressed( JNIEnv* env, jobject obj )
{
	fDebug( env, obj );
	BaseWindow::mKeys[ Input::BACK ] = true;
}

} // namespace GE 
