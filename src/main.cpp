#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

// #include <osvr/ClientKit/ClientKit.h>
// #include <osvr/ClientKit/Display.h>

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



#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include "FramebufferWindow.h"

class FBDevThread : protected Thread {
public:
	FBDevThread() {
		Start();
	}
protected:
	virtual bool run()
	{
		mInstance = Instance::Create( "test::fbdev", 42, true, "framebuffer" );
		mWindow = (ProxyWindow< FramebufferWindow >*)mInstance->CreateWindow( "", 480, 320, 0 );
		mWindow->OpenSystemFramebuffer( "/tmp/fbe_buffer" );

		Renderer2D* renderer = mInstance->CreateRenderer2D( mWindow->width(), mWindow->height() );
		Image* img = new Image( "/home/drich/prog/libge/scene/data/image.png" );

		Font* font = new Font( "/home/drich/games/StarCrash/data/RobotoCondensed-Light.ttf" );

		while ( 1 )
		{
			mWindow->Clear( 0xFF202020 );
// 			renderer->Draw( 0, 0, img );
			renderer->DrawText( 20, 20, font, 0xFFFFFFFF, "pute" );
			mWindow->SwapBuffers();

			mTicks = Time::WaitTick( 1000 / 60, mTicks );
			Time::GlobalSync();
		}
		return false;
	}
private:
	Instance* mInstance;
	ProxyWindow< FramebufferWindow >* mWindow;
	uint64_t mTicks;
};

void TEST()
{
	Instance* instance = Instance::Create( "test::vulkan", 42, true, "vulkan" );
	Window* window = instance->CreateWindow( "test::vulkan", 1280, 720, Window::Resizable );
	Font* font = new Font( "scene/Arial Unicode MS.ttf" );

	Renderer2D* renderer2d = instance->CreateRenderer2D( window->width(), window->height() );
	renderer2d->AssociateSize( window );
	renderer2d->LoadVertexShader( "shaders_vulkan/2d.vert.spv" );
	renderer2d->LoadFragmentShader( "shaders_vulkan/2d.frag.spv" );
/*
	Light* sun_light = new Light( Vector4f( 1.0, 1.0, 1.0, 1.0 ), Vector3f( 10000000.0f, 5000000.0f, 10000000.0f ), 0.0f );
	DeferredRenderer* deferredRenderer = instance->CreateDeferredRenderer( window->width(), window->height() );
	deferredRenderer->AssociateSize( window );
	deferredRenderer->setAmbientColor( Vector4f( 0.65f, 0.65f, 0.65f, 1.0f ) ); // day
	deferredRenderer->AddSunLight( sun_light );
*/
	Object* cube = instance->LoadObject( "scene/cube.obj" );
/*
	const int cubes_side = 64;
	cube->CreateInstances( cubes_side * cubes_side * cubes_side );
	for ( int k = 0; k < cubes_side; k++ ) {
		for ( int j = 0; j < cubes_side; j++ ) {
			for ( int i = 0; i < cubes_side; i++ ) {
				cube->matrix( k*cubes_side*cubes_side + j*cubes_side + i )->Translate( i, j, k );
			}
		}
	}
*/

	Renderer* render = instance->CreateRenderer();
	render->LoadVertexShader( "shaders_vulkan/basic.vert.spv" );
	render->LoadFragmentShader( "shaders_vulkan/basic.frag.spv" );
// 	render->LoadVertexShader( "shaders/basic.vert" );
// 	render->LoadFragmentShader( "shaders/basic.frag" );
	render->AddObject( cube );
	render->Compute();
// 	exit(0);




	std::list< Object* > scene_objects = Object::LoadObjects( "scene/sponza2.obj", true, instance );
	std::vector<Object*> objects{ std::begin(scene_objects), std::end(scene_objects) };
	Renderer* render2 = instance->CreateRenderer();
	render2->LoadVertexShader( "shaders_vulkan/basic.vert.spv" );
	render2->LoadFragmentShader( "shaders_vulkan/basic.frag.spv" );
// 	render2->LoadVertexShader( "shaders/basic.vert" );
// 	render2->LoadFragmentShader( "shaders/basic.frag" );
	for ( auto obj : objects ) {
		render2->AddObject( obj );
	}
	render2->Compute();

	Input* input = new Input( window );

	Camera* camera = new Camera();
	camera->setInertia( 0.999f );
	camera->LookAt( { -6.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } );

	float fps = 0.0f;
	float fps_min = 1.0e34f;
	float fps_max = 0.0f;
	float time = Time::GetSeconds();
	uint32_t img = 0;
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
		if ( input->pressed( Input::LBUTTON ) ) {
			camera->RotateH( input->cursorWarp().x, -2.0f );
			camera->RotateV( input->cursorWarp().y, -2.0f );
		}

		window->BindTarget();
		window->Clear( 0xFF202020 );

		cube->matrix()->RotateZ( 0.01f );

// 		deferredRenderer->Bind();
		render->Look( camera );
		render->Draw();
		render2->Look( camera );
		render2->Draw();
// 		deferredRenderer->Look( camera );
// 		deferredRenderer->Render();


		renderer2d->DrawText( 0, font->size() * 0, font, 0xFFFFFFFF, " FPS : " + std::to_string( (int)fps ) );
		renderer2d->DrawText( 0, font->size() * 1, font, 0xFFFFFFFF, " RAM : " + std::to_string( instance->cpuRamCounter() / 1024 ) );
		renderer2d->DrawText( 0, font->size() * 2, font, 0xFFFFFFFF, "VRAM : " + std::to_string( instance->gpuRamCounter() / 1024 ) );

		window->SwapBuffers();
		Time::GlobalSync();

		img++;
		if ( Time::GetSeconds() - time >= 1.0f ) {
			fps = img * 1.0f / ( Time::GetSeconds() - time );
			fps_min = std::min( fps_min, fps );
			fps_max = std::max( fps_max, fps );
			time = Time::GetSeconds();
			img = 0;
			printf( "fps : %.2f (%.2f - %.2f)\n", fps, fps_min, fps_max );
		}
	}

}







class PhysicsThread : protected Thread {
public:
	PhysicsThread( PhysicalGraph* world ) : mWorld( world ) {
		Start();
	}
protected:
	virtual bool run() {
		mWorld->Update( ( Time::GetTick() - mLastTicks ) / 1000.0f/*1.0f / 60.0f*/ );
		mLastTicks = mTicks;
		mTicks = Time::WaitTick( 1000 / 60, mTicks );
		Time::GlobalSync();
		return true;
	}
private:
	PhysicalGraph* mWorld;
	uint64_t mTicks;
	uint64_t mLastTicks;
};


int main( int argc, char** argv )
{
	TEST();
	return 0;

	Instance* instance = Instance::Create( "GammaEngse test", 42, true, "opengl43" );
	Window* window = instance->CreateWindow( "Hello GammaEngine !", 1280, 720, Window::Resizable );
// 	Window* window = instance->CreateWindow( "Hello GammaEngine !", 1920, 1080, Window::Fullscreen );
	Input* input = new Input( window );

	Font* font = new Font( "scene/Arial Unicode MS.ttf" );

	std::list< Object* > scene_objects = Object::LoadObjects( "scene/street.obj", true, instance );

	Object* cube = instance->LoadObject( "scene/cube.obj" );

	const int cubes_side = 4;
	Object* cubes[cubes_side*cubes_side*cubes_side];
	for ( int i = 0; i < cubes_side*cubes_side*cubes_side; i++ ) {
		cubes[i] = cube->Copy();
	}

	Image* texture = new Image( "scene/texture.png" );
	cube->setTexture( instance, 0, texture );

	Light* sun_light = new Light( Vector4f( 1.0, 1.0, 1.0, 1.0 ), Vector3f( 10000000.0f, 5000000.0f, 10000000.0f ), 0.0f );
	Light* lightm1 = new Light( Vector4f( 1.0, 1.0, 1.0, 4.0 ), Vector3f( 2.8f, 25.0f, 5.9f ) );
	Light* light0 = new Light( Vector4f( 1.0, 1.0, 1.0, 4.0 ), Vector3f( 2.8f, 0.0f, 5.9f ), Vector3f( -0.4f, 0.0f, -1.0f ), 45.0f, 70.0f );
	Light* light1 = new Light( Vector4f( 1.0, 1.0, 1.0, 4.0 ), Vector3f( 2.8f, -25.0f, 5.9f ), Vector3f( -0.4f, 0.0f, -1.0f ), 45.0f, 70.0f );
	Light* light2 = new Light( Vector4f( 1.0, 1.0, 1.0, 4.0 ), Vector3f( 2.8f, -50.0f, 5.9f ), Vector3f( -0.4f, 0.0f, -1.0f ), 45.0f, 70.0f );
	Light* light3 = new Light( Vector4f( 1.0, 1.0, 1.0, 4.0 ), Vector3f( 2.8f, -75.0f, 5.9f ), Vector3f( -0.4f, 0.0f, -1.0f ), 45.0f, 70.0f );
	Light* light4 = new Light( Vector4f( 1.0, 1.0, 1.0, 4.0 ), Vector3f( 2.8f, -100.0f, 5.9f ), Vector3f( -0.4f, 0.0f, -1.0f ), 45.0f, 70.0f );


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
				PhysicalBody* cube_body = new PhysicalBody( Vector3f( i, j, k + 42.0f ), 0.001f );
				cube_body->setBox( Vector3f( -0.5f, -0.5f, -0.5f ), Vector3f( 0.5f, 0.5f, 0.5f ) );
				cube_body->setTarget( cubes[k*cubes_side*cubes_side + j*cubes_side + i] );
				cube_body->setFriction(1.0f);
				world->AddBody( cube_body );
			}
		}
	}
	for ( auto it = scene_objects.begin(); it != scene_objects.end(); ++it ) {
		PhysicalBody* body = new PhysicalBody( (*it)->position(), 0.0f );
		body->setMesh( *it, false, true );
		body->setFriction(1.0f);
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
// 	sky->AssociateSize( window );
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

// 	osvr::clientkit::ClientContext ctx("drich.ge.vrtest");
// 	osvr::clientkit::DisplayConfig display(ctx);

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
// 		renderer->Look( camera );
// 		renderer->Draw();
		deferredRenderer->Look( camera );
		deferredRenderer->Render();
		sky->Render( camera );

/*
		disp.forEachEye([](osvr::clientkit::Eye eye) {

			/// Try retrieving the view matrix (based on eye pose) from OSVR
			double viewMat[OSVR_MATRIX_SIZE];
			eye.getViewMatrix(OSVR_MATRIX_COLMAJOR | OSVR_MATRIX_COLVECTORS,
							viewMat);
			/// Initialize the ModelView transform with the view matrix we
			/// received
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glMultMatrixd(viewMat);

			/// For each display surface seen by the given eye of the given
			/// viewer...
			eye.forEachSurface([](osvr::clientkit::Surface surface) {
				auto viewport = surface.getRelativeViewport();
				glViewport(static_cast<GLint>(viewport.left),
						static_cast<GLint>(viewport.bottom),
						static_cast<GLsizei>(viewport.width),
						static_cast<GLsizei>(viewport.height));

				/// Set the OpenGL projection matrix based on the one we
				/// computed.
				double zNear = 0.1;
				double zFar = 100;
				double projMat[OSVR_MATRIX_SIZE];
				surface.getProjectionMatrix(
					zNear, zFar, OSVR_MATRIX_COLMAJOR | OSVR_MATRIX_COLVECTORS |
									OSVR_MATRIX_SIGNEDZ | OSVR_MATRIX_RHINPUT,
					projMat);

				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				glMultMatrixd(projMat);

				/// Set the matrix mode to ModelView, so render code doesn't
				/// mess with the projection matrix on accident.
				glMatrixMode(GL_MODELVIEW);

				/// Call out to render our scene.
				renderScene();
			});
		});
*/
/*
		{
			renderer->projectionMatrix()->Perspective( 110.0f, (float)( window->width() / 2 ) / window->height(), 0.01f, 1000.0f );
			rendererPhysics->projectionMatrix()->Perspective( 110.0f, (float)( window->width() / 2 ) / window->height(), 0.01f, 1000.0f );
			sky->renderer()->projectionMatrix()->Perspective( 110.0f, (float)( window->width() / 2 ) / window->height(), 1.0f, 1378114.0f );
			glViewport( 0, 0, window->width() / 2, window->height() );
			deferredRenderer->Bind();
			scene->Draw( camera );
			deferredRenderer->Look( camera );
			deferredRenderer->Render();
			sky->Render( camera );
		}
		{
			renderer->projectionMatrix()->Perspective( 110.0f, (float)( window->width() / 2 ) / window->height(), 0.01f, 1000.0f );
			rendererPhysics->projectionMatrix()->Perspective( 110.0f, (float)( window->width() / 2 ) / window->height(), 0.01f, 1000.0f );
			sky->renderer()->projectionMatrix()->Perspective( 110.0f, (float)( window->width() / 2 ) / window->height(), 1.0f, 1378114.0f );
			glViewport( window->width() / 2, 0, window->width() / 2, window->height() );
			deferredRenderer->Bind();
			camera->setPosition( camera->position() + 0.3f * Vector3f( std::sin( camera->rotation().x ), -std::cos( camera->rotation().x ), 0.0f ) );
			scene->Draw( camera );
			deferredRenderer->Look( camera );
			deferredRenderer->Render();
			sky->Render( camera );
			camera->setPosition( camera->position() - 0.3f * Vector3f( std::sin( camera->rotation().x ), -std::cos( camera->rotation().x ), 0.0f ) );
		}
		glViewport( 0, 0, window->width(), window->height() );
*/
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
