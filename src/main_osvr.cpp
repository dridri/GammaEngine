#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include <osvr/ClientKit/ClientKit.h>
#include <osvr/ClientKit/Display.h>
#include <osvr/ClientKit/Interface.h>
#include <osvr/ClientKit/InterfaceStateC.h>

#include "Instance.h"
#include "Window.h"
#include "Scene.h"
#include "Renderer.h"
#include "Renderer2D.h"
#include "Object.h"
#include "Image.h"
#include "Font.h"
#include "Camera.h"
#include "Time.h"
#include "Input.h"
#include "PhysicalBody.h"
#include "PhysicalGraph.h"
#include "Thread.h"
#include "Socket.h"
#include "Light.h"
#include "DeferredRenderer.h"
#include "SkyRenderer.h"
#include "Sound.h"
#include "Music.h"
#include "Debug.h"

extern "C" void glViewport(int, int, int, int);
using namespace GE;

int main( int argc, char** argv )
{
	Instance* instance = Instance::Create( "GammaEngse test", 42, true, "opengl43" );
	Window* window = instance->CreateWindow( "Hello GammaEngine !", 1920, 1080, Window::Fullscreen );
	Input* input = new Input( window );

	Font* font = new Font( "scene/Arial Unicode MS.ttf" );

	std::list< Object* > scene_objects = Object::LoadObjects( "scene/street.obj", true, instance );

	Object* cube = instance->LoadObject( "scene/cube.obj" );

	const int cubes_side = 8;
	Object* cubes[cubes_side*cubes_side*cubes_side];
	for ( int i = 0; i < cubes_side*cubes_side*cubes_side; i++ ) {
		cubes[i] = cube->Copy();
	}

	Image* texture = new Image( "scene/texture.png" );
	cube->setTexture( instance, 0, texture );

	Light* sun_light = new Light( Vector4f( 1.0, 1.0, 1.0, 1.0 ), Vector3f( 10000000.0f, 5000000.0f, 10000000.0f ), 0.0f );
	Light* lightm1 = new Light( Vector4f( 1.0, 1.0, 1.0, 4.0 ), Vector3f( 2.8f, 25.0f, 5.9f ) );
	Light* light0 = new Light( Vector4f( 1.0, 1.0, 1.0, 8.0 ), Vector3f( 2.8f, 0.0f, 5.9f ), Vector3f( -0.4f, 0.0f, -1.0f ), 30.0f, 50.0f );
	Light* light1 = new Light( Vector4f( 1.0, 1.0, 1.0, 8.0 ), Vector3f( 2.8f, -25.0f, 5.9f ), Vector3f( -0.4f, 0.0f, -1.0f ), 30.0f, 50.0f );
	Light* light2 = new Light( Vector4f( 1.0, 1.0, 1.0, 8.0 ), Vector3f( 2.8f, -50.0f, 5.9f ), Vector3f( -0.4f, 0.0f, -1.0f ), 30.0f, 50.0f );
	Light* light3 = new Light( Vector4f( 1.0, 1.0, 1.0, 8.0 ), Vector3f( 2.8f, -75.0f, 5.9f ), Vector3f( -0.4f, 0.0f, -1.0f ), 30.0f, 50.0f );
	Light* light4 = new Light( Vector4f( 1.0, 1.0, 1.0, 8.0 ), Vector3f( 2.8f, -100.0f, 5.9f ), Vector3f( -0.4f, 0.0f, -1.0f ), 30.0f, 50.0f );


	Renderer* renderer = instance->CreateRenderer();
	renderer->LoadVertexShader( "shaders/basic.vert" );
	renderer->LoadFragmentShader( "shaders/basic.frag" );
	for ( auto it = scene_objects.begin(); it != scene_objects.end(); ++it ) {
		renderer->AddObject( *it );
	}
	renderer->Compute();

	Renderer2D* renderer2d = instance->CreateRenderer2D();
	renderer2d->AssociateSize( window );

	DeferredRenderer* deferredRenderer = instance->CreateDeferredRenderer( window->width(), window->height() );
	deferredRenderer->AssociateSize( window );
// 	deferredRenderer->setAmbientColor( Vector4f( 0.15f, 0.15f, 0.15f, 1.0f ) ); // night
	deferredRenderer->setAmbientColor( Vector4f( 0.65f, 0.65f, 0.65f, 1.0f ) ); // day
	deferredRenderer->AddSunLight( sun_light );

	deferredRenderer->AddLight( sun_light );
	deferredRenderer->AddLight( lightm1 );
	deferredRenderer->AddLight( light0 );
	deferredRenderer->AddLight( light1 );
	deferredRenderer->AddLight( light2 );
	deferredRenderer->AddLight( light3 );
	deferredRenderer->AddLight( light4 );

	SkyRenderer* sky = new SkyRenderer( instance, 1378114.0f );
	sky->AddLight( sun_light );

	Scene* scene = new Scene();
	scene->AddRenderer( renderer );

	Camera* camera = new Camera();
	camera->setInertia( 0.999f );
	camera->LookAt( { 0.0f, 0.0f, 1.8f }, { 0.0f, 0.0f, 0.0f } );
	Camera* osvr_camera = new Camera( Camera::Free );

	float fps = 0.0f;
	float fps_min = 1.0e34f;
	float fps_max = 0.0f;
	float time = Time::GetSeconds();
	float sun_angle = 0.0f;
	uint32_t img = 0;

	osvr::clientkit::ClientContext context("drich.ge.vrtest");
	osvr::clientkit::DisplayConfig display(context);
	if ( !display.valid() ) {
		printf( "error\n" );
		return 0;
	}
	while (!display.checkStartup()) {
		context.update();
	}
    osvr::clientkit::Interface head = context.getInterface("/me/head");

	while ( 1 ) {
		input->Update();
		if ( input->pressed( 'E' ) ) {
			camera->WalkForward( 10.0 );
		}
		if ( input->pressed( 'D' ) ) {
			camera->WalkBackward( 10.0 );
		}
		if ( input->pressed( 'S' ) ) {
			camera->WalkLeft( 10.0 );
		}
		if ( input->pressed( 'F' ) ) {
			camera->WalkRight( 10.0 );
		}
		if ( input->pressed( ' ' ) ) {
			camera->Translate( { 0.0f, 0.0f, (float)( 10.0 * Time::Delta() ) } );
		}
		if ( input->pressed( 'C' ) ) {
			camera->Translate( { 0.0f, 0.0f, (float)( -10.0 * Time::Delta() ) } );
		}
		if ( input->pressed( 'R' ) ) {
			sun_angle += 0.01f;
			Vector3f pos = sun_light->position();
			pos.x = 10000000.0f * std::cos( sun_angle );
			pos.z = 10000000.0f * std::sin( sun_angle );
			if ( pos.z <= 0.0f ) {
				sun_light->setColor( Vector4f( 0.0, 0.0, 0.0, 1.0 ) );
			} else {
				sun_light->setColor( Vector4f( 1.0, 1.0, 1.0, 1.0 ) );
			}
			sun_light->setPosition( pos );
			deferredRenderer->Update( sun_light );
			float amb = 0.1f + 0.4f * std::max( pos.z, 0.0f ) / 10000000.0f;
			deferredRenderer->setAmbientColor( Vector4f( amb, amb, amb, 1.0f ) );
		}
		if ( input->pressed( 'T' ) ) {
			sun_angle -= 0.01f;
			Vector3f pos = sun_light->position();
			pos.x = 10000000.0f * std::cos( sun_angle );
			pos.z = 10000000.0f * std::sin( sun_angle );
			if ( pos.z <= 0.0f ) {
				sun_light->setColor( Vector4f( 0.0, 0.0, 0.0, 1.0 ) );
			} else {
				sun_light->setColor( Vector4f( 1.0, 1.0, 1.0, 1.0 ) );
			}
			sun_light->setPosition( pos );
			deferredRenderer->Update( sun_light );
			float amb = 0.1f + 0.4f * std::max( pos.z, 0.0f ) / 10000000.0f;
			deferredRenderer->setAmbientColor( Vector4f( amb, amb, amb, 1.0f ) );
		}
		
		if ( input->pressed( Input::LBUTTON ) ) {
// 			camera->RotateH( input->cursorWarp().x, -2.0f );
// 			camera->RotateV( input->cursorWarp().y, -2.0f );
		}

		window->Clear( 0xFF202020 );
		window->BindTarget();

		context.update();
		OSVR_PoseState state;
		OSVR_TimeValue timestamp;
		OSVR_ReturnCode ret = osvrGetPoseState( head.get(), &timestamp, &state );
		Quaternion rot = Quaternion( state.rotation.data[0], state.rotation.data[1], state.rotation.data[2], state.rotation.data[3] );
		Vector4f vec = { 1.0f, 0.0f, 0.0f, 0.0f };
		vec = rot.matrix() * vec;
		float rotH = std::acos( vec.x );
		float rotV = std::asin( vec.y );
		camera->setRotation( rotH, rotV );

		display.forEachEye([window,camera,osvr_camera,scene,renderer,sky,deferredRenderer](osvr::clientkit::Eye eye) {
			Matrix eyeMat, viewMat;
			eye.getViewMatrix( OSVR_MATRIX_COLMAJOR | OSVR_MATRIX_COLVECTORS, eyeMat.data() );
// 			eyeMat.RotateX( -M_PI / 2.0f );
			Camera tmp;
			tmp.setPosition( camera->position() );
			memcpy( viewMat.data(), tmp.data(), sizeof(float)*16 );
			osvr_camera->setMatrix( eyeMat * viewMat );

			eye.forEachSurface( [window,osvr_camera,scene,renderer,sky,deferredRenderer]( osvr::clientkit::Surface surface ) {
				auto viewport = surface.getRelativeViewport();
				glViewport( static_cast<int>(viewport.left), static_cast<int>(viewport.bottom), static_cast<size_t>(viewport.width), static_cast<size_t>(viewport.height) );

				/// Set the OpenGL projection matrix based on the one we
				/// computed.
				double zNear = 0.1;
				double zFar = 100;
				double projMat[OSVR_MATRIX_SIZE];
				surface.getProjectionMatrix( 0.01f, 1000.0f, OSVR_MATRIX_COLMAJOR | OSVR_MATRIX_COLVECTORS | OSVR_MATRIX_SIGNEDZ | OSVR_MATRIX_RHINPUT, deferredRenderer->projectionMatrix()->data() );
				surface.getProjectionMatrix( 0.01f, 1000.0f, OSVR_MATRIX_COLMAJOR | OSVR_MATRIX_COLVECTORS | OSVR_MATRIX_SIGNEDZ | OSVR_MATRIX_RHINPUT, renderer->projectionMatrix()->data() );
				surface.getProjectionMatrix( 1.0f, 1378124.0f, OSVR_MATRIX_COLMAJOR | OSVR_MATRIX_COLVECTORS | OSVR_MATRIX_SIGNEDZ | OSVR_MATRIX_RHINPUT, sky->renderer()->projectionMatrix()->data() );
// 				renderer->projectionMatrix()->Perspective( 100.0f, (float)(window->width() / 2) / window->height(), 0.01f, 1000.0f );

				deferredRenderer->Bind();
				scene->Draw( osvr_camera );
				deferredRenderer->Look( osvr_camera );
				deferredRenderer->Render();
				sky->Render( osvr_camera );
			});
		});

		glViewport( 0, 0, window->width(), window->height() );

		renderer2d->DrawText( 0, font->size() * 0, font, 0xFFFFFFFF, " FPS : " + std::to_string( (int)fps ) );
		renderer2d->DrawText( 0, font->size() * 1, font, 0xFFFFFFFF, " RAM : " + std::to_string( instance->cpuRamCounter() / 1024 ) );
		renderer2d->DrawText( 0, font->size() * 2, font, 0xFFFFFFFF, "VRAM : " + std::to_string( instance->gpuRamCounter() / 1024 ) );

		window->SwapBuffers();
		Time::GlobalSync();

		img++;
		if ( Time::GetSeconds() - time > 1.0f ) {
			fps = img * 1.0f / ( Time::GetSeconds() - time );
			fps_min = std::min( fps_min, fps );
			fps_max = std::max( fps_max, fps );
			time = Time::GetSeconds();
			img = 0;
		}
	}

	return 0;
}
