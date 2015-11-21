#include <unistd.h>
#include <string.h>
#include <iostream>
#include "Debug.h"
#include "Instance.h"
#include "Window.h"
#include "Scene.h"
#include "Renderer.h"
#include "Object.h"
#include "Image.h"
#include "Camera.h"
#include "Time.h"
#include "Input.h"
#include "PhysicalBody.h"
#include "PhysicalGraph.h"

using namespace GE;

int main2();

float collide_x = 0.0f;
float collide_y = 0.0f;
float collide1_x = 0.0f;
float collide1_y = 0.0f;
float collide2_x = 0.0f;
float collide2_y = 0.0f;
/*
#include <libge/libge.h>
void main2d(){
	geInit();
	geCreateMainWindow( "GE 2D", 1280, 720, 0 );

	Instance* instance = Instance::Create( "GammaEngine test", 42 );
	PhysicalBody* physics = new PhysicalBody( { 0.0f, 0.0f, 0.0f }, 1.0f );
	physics->setBoundingBox( { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } );
// 	PhysicalBody* physics2 = new PhysicalBody( { 0.0f, 3.0f, 0.0f }, 1.0f );
	PhysicalBody* physics2 = new PhysicalBody( { 0.5f, 3.0f, 0.0f }, 1.0f );
// 	PhysicalBody* physics2 = new PhysicalBody( { 0.95f, 3.0f, 0.0f }, 1.0f );
	physics2->setBoundingBox( { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } );
	PhysicalGraph* graph = new PhysicalGraph();
	graph->AddBody( physics );
	graph->AddBody( physics2 );

	uint32_t time = 1000;
	ge_Keys* keys = geCreateKeys();

	while(1){
// 		Time::GlobalSync();
		geReadKeys( keys );
		if ( keys->pressed[ GEK_LEFT ] && time > 1000 + 10 ) {
			time -= 10;
		}
		if ( keys->pressed[ GEK_RIGHT ] ) {
			time += 10;
		}
		Time::setTime( time );

		geClearScreen();

		physics->ResetForces();
		physics->ResetTorque();
		physics2->ResetForces();
		physics2->ResetTorque();
// 		physics->ApplyTorque( Vector3f( 0.0f, 0.0f, 0.1f ) * 1.0f );
// 		physics2->ApplyTorque( Vector3f( 0.0f, 0.0f, 0.02f ) * 1.0f );
		graph->Update();

// 		geDrawOffset( 256, 512 );

// 		geDrawImageRotated( physics->position().y * 128 - 64, physics->position().x * 128 - 64 );
		geMatrixMode( GE_MATRIX_VIEW );
		geLoadIdentity();
		geScale( 50.0f, 50.0f, 50.0f );
		geTranslate( 5.0f, 5.0f, 0.0f );
		geMatrixMult( physics->matrix().constData() );
		geScale( 0.5f, 0.5f, 0.5f );
// 		geFillRectScreen( -64, -64, 128, 128, 0xFFFFFFFF );
		geFillRectScreen( -1, -1, 2, 2, 0xFF606060 );
		geLoadIdentity();
		geScale( 50.0f, 50.0f, 50.0f );
		geTranslate( 5.0f, 5.0f, 0.0f );
		geMatrixMult( physics2->matrix().constData() );
		geScale( 0.5f, 0.5f, 0.5f );
// 		geFillRectScreen( -64, -64, 128, 128, 0xFFFFFFFF );
		geFillRectScreen( -1, -1, 2, 2, 0xFF606060 );

		uint32_t nVerts = 0;
		Vector3f* baseVerts = graph->collider()->minkowskiDifference( physics, physics2, &nVerts );
		geMatrixMode( GE_MATRIX_VIEW );
		geLoadIdentity();
// 		geScale( 50.0f, 50.0f, 50.0f );
		geTranslate( 250.0f, 250.0f, 0.0f );
// 		geScale( 0.5f, 0.5f, 0.5f );
		for( uint32_t i = 0; i < nVerts; i++ ) {
			geFillRectScreen( baseVerts[i].x * 50.0f - 1, baseVerts[i].y * 50.0f - 1, 2, 2, 0xFFFFFFFF );
		}
		instance->Free( baseVerts );

		geMatrixMode( GE_MATRIX_VIEW );
		geLoadIdentity();
		geDrawLineScreen( 0, 250, 1280, 250, 0xFF00007F );
		geDrawLineScreen( 250, 0, 250, 720, 0xFF00007F );
		if( collide_x != 0.0f && collide_y != 0.0f ) {
			geDrawLineScreen( 0, 250 + collide1_y * 50.0, 1280, 250 + collide1_y * 50.0, 0xFFFF0000 );
			geDrawLineScreen( 250 + collide1_x * 50.0f, 0, 250 + collide1_x * 50.0f, 720, 0xFFFF0000 );
			geDrawLineScreen( 0, 250 + collide2_y * 50.0, 1280, 250 + collide2_y * 50.0, 0xFF00FF00 );
			geDrawLineScreen( 250 + collide2_x * 50.0f, 0, 250 + collide2_x * 50.0f, 720, 0xFF00FF00 );
			geDrawLineScreen( 0, 250 + collide_y * 50.0, 1280, 250 + collide_y * 50.0, 0xFF0000FF );
			geDrawLineScreen( 250 + collide_x * 50.0f, 0, 250 + collide_x * 50.0f, 720, 0xFF0000FF );
		}
		geSwapBuffers();
	}
}
*/

int main( int argc, char** argv )
{
	chdir("..");
// 	main2d();
// 	return 0;

	Instance* instance = Instance::Create( "GammaEngine test", 42 );
	Window* window = instance->CreateWindow( "Hello GammaEngine !", 1280, 720 );
	Input* input = new Input( window );

	Object* cube = instance->LoadObject( "scene/cube.obj" );
	Object* cube2 = instance->LoadObject( "scene/cube.obj" );
	Object* cube3 = instance->LoadObject( "scene/cube.obj" );
	Object* ground = instance->LoadObject( "scene/cube.obj" );

	PhysicalBody* physics = new PhysicalBody( { 0.0f, 0.0f, 0.0f }, 1.0f );
	physics->setBoundingBox( { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } );

	PhysicalBody* physics2 = new PhysicalBody( { 0.0f, 3.0f, 0.5f }, 1.0f );
	physics2->setBoundingBox( { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } );

	PhysicalBody* physics3 = new PhysicalBody( { 0.0f, 4.0f, 0.0f }, 2.0f );
	physics3->setBoundingBox( { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } );

	PhysicalBody* pground = new PhysicalBody( { 0.0f, 0.0f, -4.0f }, 100.0f );
	pground->setBoundingBox( { -50.0f, -50.0f, -0.5f }, { 50.0f, 50.0f, 0.5f } );

	PhysicalGraph* graph = new PhysicalGraph();
	graph->AddBody( physics );
	graph->AddBody( physics2 );
// 	graph->AddBody( physics3 );
	graph->AddBody( pground );
// 	Image* image = new Image( "scene/texture.png" );

	Renderer* renderer = instance->CreateRenderer();
	renderer->LoadVertexShader( "shaders/basic.vert" );
	renderer->LoadFragmentShader( "shaders/basic.frag" );
	renderer->AddObject( cube );
	renderer->AddObject( cube2 );
// 	renderer->AddObject( cube3 );
	renderer->AddObject( ground );
	renderer->Compute();
/*
	uint32_t nVerts = 0;
	Vector3f* baseVerts = graph->collider()->minkowskiDifference( physics, physics2, &nVerts );
	Vertex* verts = (Vertex*)instance->Malloc( sizeof(Vertex) * nVerts );
	gDebug() << "nVerts = " << nVerts << "\n";
	for( uint32_t i = 0; i < nVerts; i++ ) {
		verts[i].x = baseVerts[i].x;
		verts[i].y = baseVerts[i].y;
		verts[i].z = baseVerts[i].z;
		verts[i].color[0] = 1.0f;
		verts[i].color[1] = 1.0f;
		verts[i].color[2] = 1.0f;
		verts[i].color[3] = 1.0f;
	}
	Object* test_box = instance->CreateObject( verts, nVerts );
	Renderer* renderer2 = instance->CreateRenderer();
	renderer2->setRenderMode( VK_TOPOLOGY_POINT_LIST );
	renderer2->LoadVertexShader( "shaders/basic.vert" );
	renderer2->LoadFragmentShader( "shaders/basic.frag" );
	renderer2->AddObject( test_box );
	renderer2->Compute();
*/
	Scene* scene = new Scene();
	scene->AddRenderer( renderer );
// 	scene->AddRenderer( renderer2 );

	Camera* camera = new Camera();
	camera->setInertia( 0.999f );
	camera->LookAt( { -6.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } );

	float fps = 0.0f;
	float time = Time::GetSeconds();
	uint32_t img = 0;

	while ( 1 ) {
		physics->ResetForces();
		physics->ResetTorque();
		physics2->ResetForces();
		physics2->ResetTorque();
		physics3->ResetForces();
		physics3->ResetTorque();
		pground->ResetForces();
		pground->ResetTorque();
// 		physics->ApplyGravity( earth );
// 		earth->ApplyGravity( physics );
		physics->ApplyTorque( Vector3f( 0.01f, 0.05f, 0.0f ) *1.0f );
		physics2->ApplyTorque( Vector3f( 0.0f, -0.05f, 0.001f ) *1.0f );
// 		earth->ApplyTorque( Vector3f( -0.01f, 0.005f, 0.008f ) * 1e12f );

		input->Update();
		if ( input->pressed( 'Z' ) ) {
			camera->WalkForward( 5.0 );
		}
		if ( input->pressed( 'S' ) ) {
			camera->WalkBackward( 5.0 );
		}
		if ( input->pressed( 'Q' ) ) {
			camera->WalkLeft( 5.0 );
		}
		if ( input->pressed( 'D' ) ) {
			camera->WalkRight( 5.0 );
		}
		if ( input->pressed( ' ' ) ) {
			camera->Translate( { 0.0f, 0.0f, (float)( 5.0 * Time::Delta() ) } );
		}
		if ( input->pressed( 'C' ) ) {
			camera->Translate( { 0.0f, 0.0f, (float)( -5.0 * Time::Delta() ) } );
		}
		if ( input->pressed( Input::LBUTTON ) ) {
			camera->RotateH( input->cursorWarp().x, -2.0f );
			camera->RotateV( input->cursorWarp().y, -2.0f );
		}
		if ( input->pressed( Input::RBUTTON ) ) {
			physics->ApplyForce( camera->direction() * 1.0f );
		}

// 		graph->Update();
/*
		baseVerts = graph->collider()->minkowskiDifference( physics, physics2, &nVerts );
		for( uint32_t i = 0; i < nVerts; i++ ) {
			test_box->vertices()[i].x = baseVerts[i].x;
			test_box->vertices()[i].y = baseVerts[i].y;
			test_box->vertices()[i].z = baseVerts[i].z;
		}
		instance->Free( baseVerts );
		test_box->UpdateVertices( instance, test_box->vertices(), 0, nVerts );
*/
		cube->matrix()->Identity();
		cube2->matrix()->Identity();
		cube3->matrix()->Identity();
		ground->matrix()->Identity();

		cube->matrix()->operator*=( physics->matrix() );
		cube2->matrix()->operator*=( physics2->matrix() );
		cube3->matrix()->operator*=( physics3->matrix() );
		ground->matrix()->operator*=( pground->matrix() );

		ground->matrix()->Scale( 100.0f, 100.0f, 1.0f );
		ground->matrix()->RotateX( M_PI );

		window->Clear( 0xFF202020 );
		window->BindTarget();

		scene->Draw( camera );

		window->SwapBuffers();

// 		ticks = Time::WaitTick( 1000 / 60, ticks );
		Time::GlobalSync();

		img++;
		if ( Time::GetSeconds() - time > 0.2f ) {
			fps = img * 1.0f / ( Time::GetSeconds() - time );
			time = Time::GetSeconds();
			img = 0;
			printf("FPS : %.2f\n", fps);
// 			printf(" RAM : %lu kB\n", instance->cpuRamCounter() / 1024);
// 			printf("VRAM : %lu kB\n", instance->gpuRamCounter() / 1024);
		}
// 		printf("FPS : %.2f\n", fps);
	}

	return 0;
}
