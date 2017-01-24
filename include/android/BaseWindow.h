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

#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include "Vector.h"
#include <string>
#include <thread>

// #ifdef BASEWINDOW_CPP

	#include <android/sensor.h>
	#include <android/log.h>
	#include <android/native_activity.h>
	#include <android/native_window_jni.h>
	#include <android/window.h>
	#include <EGL/egl.h>
	#include <jni.h>
	#include <android_native_app_glue.h>


namespace GE {

class Instance;

	typedef struct ATouch {
		bool used;
		int id;
		int action;
		float x, y;
		float force;
	} ATouch;

	typedef struct Engine {
		JNIEnv* env;
		jobject aJavaSurface;
		struct android_app* app;
		ANativeWindow* aSurface;
		bool hasSurface;
		int ofsx;
		int ofsy;
		int fullscreen;
		int aflags;

		ASensorManager* sensorManager;
		const ASensor* accelerometerSensor;
		ASensorEventQueue* sensorEventQueue;

		EGLDisplay display;
		EGLSurface surface;
		EGLContext context;
		EGLConfig config;
		EGLint format;
		int32_t width;
		int32_t height;

		bool mainWindowCreated;
		bool gotFocus;
		bool justLostFocus;
		ATouch touches[16];
		int cursorId;
		bool last_cPress;
	} Engine;
/*
#else // BASEWINDOW_CPP

	#define Engine void
	struct android_app;
	#define AInputEvent void

#endif // BASEWINDOW_CPP
*/
class BaseWindow
{
public:
	BaseWindow( Instance* instance, const std::string& title, int width, int height, uint32_t flags );
	~BaseWindow();

	uint32_t width();
	uint32_t height();
	Vector2i& cursor();
	Vector2i& cursorWarp();

	void SwapBuffersBase();
	float fps() const;

	static void ShowInterstitialAd();
	static void AndroidInit( struct android_app* state );
	static void AndroidExit( int retcode = 0 );
	static Engine* androidEngine() { return mEngine; }
	static struct android_app* androidState() { return mState; }
	static bool mAdsVisible;
	static bool mExitApp;
	static bool mKeys[512];

protected:
	Instance* mInstance;
	uint32_t mWidth;
	uint32_t mHeight;
	bool mHasResized;
	uint64_t mWindow;
	std::thread* mEventThread;
	static Vector2i mCursor;
	static Vector2i mCursorWarp;

	float mFps;
	int mFpsImages;
	uint64_t mFpsTimer;

protected://private:
	bool mClosing;
	static Engine* mEngine;
	static uint64_t mPauseTime;
	static bool mExiting;

private:
	static void PollEvents();
	static void engine_term_display( Engine* engine );
	static void engine_handle_cmd( struct android_app* app, int32_t cmd );
	static int32_t engine_handle_input( struct android_app* app, AInputEvent* event );
	static struct android_app* mState;
};


} // namespace GE

#endif // BASEWINDOW_H
 