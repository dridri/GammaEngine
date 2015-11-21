#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

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

using namespace GE;

class PhysicsThread : protected Thread {
public:
	PhysicsThread( PhysicalGraph* world ) : mWorld( world ) {
		Start();
	}
protected:
	virtual bool run() {
		mWorld->Update( 1.0f / 60.0f );
		mTicks = Time::WaitTick( 1000 / 60, mTicks );
		return true;
	}
private:
	PhysicalGraph* mWorld;
	uint64_t mTicks;
};


int main( int argc, char** argv )
{
	srand( time(nullptr) );

	Instance* instance = Instance::Create( "GammaEngine test", 42, true, "opengl43" );
	Window* window = instance->CreateWindow( "Hello GammaEngine !", 1280, 720, Window::Resizable );
	Input* input = new Input( window );

	Font* font = new Font( "scene/Arial Unicode MS.ttf" );

	std::list< Object* > scene_objects = Object::LoadObjects( "scene/street.obj", true, instance );

	Object* cube = instance->LoadObject( "scene/cube.obj" );

	const int cubes_side = 6;
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

	Renderer* rendererPhysics = instance->CreateRenderer();
	rendererPhysics->LoadVertexShader( "shaders/basic.vert" );
	rendererPhysics->LoadFragmentShader( "shaders/basic.frag" );
// 	rendererPhysics->AddObject( cube );
	for ( int i = 0; i < cubes_side*cubes_side*cubes_side; i++ ) {
		rendererPhysics->AddObject( cubes[i] );
	}
	rendererPhysics->Compute();

	PhysicalGraph* world = new PhysicalGraph( instance, Vector3f( 0.0f, 0.0f, -9.81f ) );
// 	PhysicalBody* cube_body = new PhysicalBody( Vector3f( 0.0f, 0.0f, 50.0f ), 1.0f );
// 	cube_body->setBox( Vector3f( -0.5f, -0.5f -0.5f ), Vector3f( 0.5f, 0.5f, 0.5f ) );
// 	cube_body->setTarget( cube );
// 	world->AddBody( cube_body );
	
	for ( int k = 0; k < cubes_side; k++ ) {
		for ( int j = 0; j < cubes_side; j++ ) {
			for ( int i = 0; i < cubes_side; i++ ) {
				PhysicalBody* cube_body = new PhysicalBody( Vector3f( i, j, k + 42.0f ), 10.0f );
				cube_body->setBox( Vector3f( -0.5f, -0.5f, -0.5f ), Vector3f( 0.5f, 0.5f, 0.5f ) );
				cube_body->setTarget( cubes[k*cubes_side*cubes_side + j*cubes_side + i] );
				world->AddBody( cube_body );
			}
		}
	}
	for ( auto it = scene_objects.begin(); it != scene_objects.end(); ++it ) {
		PhysicalBody* body = new PhysicalBody( (*it)->position(), 0.0f );
		body->setMesh( *it, false, true );
		world->AddBody( body );
	}

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
	sky->AssociateSize( window );
	sky->AddLight( sun_light );

	Scene* scene = new Scene();
	scene->AddRenderer( renderer );
	scene->AddRenderer( rendererPhysics );

	Camera* camera = new Camera();
	camera->setInertia( 0.999f );
	camera->LookAt( { -6.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } );

	float fps = 0.0f;
	float fps_min = 1.0e34f;
	float fps_max = 0.0f;
	float time = Time::GetSeconds();
	float sun_angle = 0.0f;
	uint32_t img = 0;

	PhysicsThread* physicsThread = new PhysicsThread( world );

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
			camera->RotateH( input->cursorWarp().x, -2.0f );
			camera->RotateV( input->cursorWarp().y, -2.0f );
		}

		window->Clear( 0xFF202020 );
		window->BindTarget();

		renderer->projectionMatrix()->Perspective( 60.0f, (float)window->width() / window->height(), 0.01f, 1000.0f );

		deferredRenderer->Bind();
		scene->Draw( camera );
		deferredRenderer->Look( camera );
		deferredRenderer->Render();
		sky->Render( camera );

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
