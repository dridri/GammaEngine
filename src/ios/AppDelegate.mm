#include "Input.h"
#include "BaseWindow.h"
#include "Vector.h"
#import "AppDelegate.h"
#include <fstream>

int _ge_main(int, char**);

static AppDelegate* sApp = nil;
EAGLContext* _ge_eagl_window_context = nil;
static GLKView* view = 0;
static uint8_t _ge_ios_keys[512] = { 0 };

static NSString* adMobPubId = nil;
static UIViewController* adMobViewController = nil;

GLuint _ge_ios_defaultFramebuffer = 0;
GLuint _ge_ios_colorRenderbuffer = 0;
GLuint _ge_ios_depthRenderbuffer = 0;

GLint _ge_ios_framebufferWidth = 0;
GLint _ge_ios_framebufferHeight = 0;

int _ge_argc = 0;
char** _ge_argv = 0;

static int _ge_mouse_x = 0;
static int _ge_mouse_y = 0;
static int _ge_mouse_wrap_x = 0;
static int _ge_mouse_wrap_y = 0;

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	sApp = self;
	[application setStatusBarHidden: NO];
	[application setStatusBarStyle: UIStatusBarStyleLightContent];
	[[UIApplication sharedApplication] setStatusBarHidden: NO];
	[[UIApplication sharedApplication] setStatusBarStyle: UIStatusBarStyleLightContent];

    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.interstitial = nil;

	_ge_eagl_window_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
	view = [[GLKView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	view.context = _ge_eagl_window_context;
	view.delegate = self;
	[self.window addSubview:view];
	
	
	CAEAGLLayer * const eaglLayer = (CAEAGLLayer*)view.layer;
	eaglLayer.opaque = YES;
	eaglLayer.contentsScale = 1.0f;

	[EAGLContext setCurrentContext:_ge_eagl_window_context];

	glGenFramebuffers( 1, &_ge_ios_defaultFramebuffer );
	glBindFramebuffer( GL_FRAMEBUFFER, _ge_ios_defaultFramebuffer );
	
	glGenRenderbuffers( 1, &_ge_ios_colorRenderbuffer );
	glBindRenderbuffer( GL_RENDERBUFFER, _ge_ios_colorRenderbuffer );
	[_ge_eagl_window_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];
	glGetRenderbufferParameteriv( GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_ge_ios_framebufferWidth );
	glGetRenderbufferParameteriv( GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_ge_ios_framebufferHeight );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _ge_ios_colorRenderbuffer );

	gDebug() << "Resolution : " << _ge_ios_framebufferWidth << ", " << _ge_ios_framebufferHeight << "\n";

	glGenRenderbuffers( 1, &_ge_ios_depthRenderbuffer );
	glBindRenderbuffer( GL_RENDERBUFFER, _ge_ios_depthRenderbuffer );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _ge_ios_framebufferWidth, _ge_ios_framebufferHeight );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _ge_ios_depthRenderbuffer );

    self.window.backgroundColor = [UIColor blackColor];
    [self.window makeKeyAndVisible];

 	[self performSelector:@selector(runMain:) withObject:nil afterDelay:0.2f];

    return YES;
}


- (void)SetAdmobPublisherId:(NSString*)pubid
{
	fDebug( pubid );
	adMobPubId = pubid;
	if ( self.interstitial == nil ) {
		self.interstitial = [ [GADInterstitial alloc] initWithAdUnitID: adMobPubId ];
		self.interstitial.delegate = self;
		GADRequest* request = [ GADRequest request ];
		request.testDevices = @[ kGADSimulatorID ];
		[ self.interstitial loadRequest:request ];
	}

	if ( adMobViewController == nil ) {
		adMobViewController = [ [UIViewController alloc] init ];
		adMobViewController.view.backgroundColor = [ UIColor blackColor ];
	}
}


- (void)ShowInterstitialAd
{
	fDebug0();
	if ( self.interstitial != nil && [ self.interstitial isReady ] ) {
		gDebug() << "interstitial ad ready\n";
		self.window.rootViewController = adMobViewController;
//		[self.window makeKeyAndVisible];
		[ self.interstitial presentFromRootViewController: adMobViewController ];
	} else {
		gDebug() << "interstitial ad not ready\n";
	}
}


- (void)interstitialDidDismissScreen:(GADInterstitial *)interstitial
{
	fDebug( interstitial );

	gDebug() << "Requesting new ad\n";
	self.interstitial = [ [GADInterstitial alloc] initWithAdUnitID: adMobPubId ];
	self.interstitial.delegate = self;
	GADRequest* request = [ GADRequest request ];
	request.testDevices = @[ kGADSimulatorID ];
	[ self.interstitial loadRequest:request ];

	self.window.rootViewController = nil;
	[self.window addSubview:view];
//	[self.window makeKeyAndVisible];
}

-(float) statusBarHeight
{
	CGSize statusBarSize = [[UIApplication sharedApplication] statusBarFrame].size;
	return MIN(statusBarSize.width, statusBarSize.height);
}



- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}


- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}


- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}


- (void)applicationWillTerminate:(UIApplication *)application
{
    //Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}


- (void)runMain:(id)sender
{
	
	[EAGLContext setCurrentContext:_ge_eagl_window_context];

	glBindFramebuffer( GL_FRAMEBUFFER, _ge_ios_defaultFramebuffer );
	_ge_main( _ge_argc, _ge_argv );
}


#pragma mark - GLKViewDelegate
- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
}


- (void) touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
	CGPoint startPoint = [[touches anyObject] locationInView:view];
	float x = startPoint.x;
	float y = startPoint.y;
	x = x * (float)_ge_ios_framebufferWidth / (float)[[UIScreen mainScreen] bounds].size.width;
	y = y * (float)_ge_ios_framebufferHeight / (float)[[UIScreen mainScreen] bounds].size.height;
	_ge_mouse_x = x;
	_ge_mouse_y = y;
	BaseWindow::iOSKeys()[Input::LBUTTON] = 1;
}


- (void) touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
	CGPoint startPoint = [[touches anyObject] locationInView:view];
	float x = startPoint.x;
	float y = startPoint.y;
	x = x * (float)_ge_ios_framebufferWidth / (float)[[UIScreen mainScreen] bounds].size.width;
	y = y * (float)_ge_ios_framebufferHeight / (float)[[UIScreen mainScreen] bounds].size.height;
	_ge_mouse_x = x;
	_ge_mouse_y = y;
}


- (void) touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
	CGPoint startPoint = [[touches anyObject] locationInView:view];
	float x = startPoint.x;
	float y = startPoint.y;
	x = x * (float)_ge_ios_framebufferWidth / (float)[[UIScreen mainScreen] bounds].size.width;
	y = y * (float)_ge_ios_framebufferHeight / (float)[[UIScreen mainScreen] bounds].size.height;
	_ge_mouse_x = x;
	_ge_mouse_y = y;
	BaseWindow::iOSKeys()[Input::LBUTTON] = 0;
}


void _ge_iOSOpenWindow( int* width, int* height, Window::Flags flags )
{
	(void)flags;
	*width = _ge_ios_framebufferWidth;
	*height = _ge_ios_framebufferHeight;
}


void _ge_iOSSetAdmobPublisherID( const char* id )
{
	fDebug( id );
	[ sApp SetAdmobPublisherId: [ NSString stringWithUTF8String: id ] ];
}


void _ge_iOSShowInterstitialAd()
{
	[ sApp ShowInterstitialAd ];
}


int _ge_iOSStatusBarHeight()
{
	return [ sApp statusBarHeight ];
}


void _ge_iOSFramebufferUseDefault()
{
	glBindFramebuffer( GL_FRAMEBUFFER, _ge_ios_defaultFramebuffer );
}


void _ge_iOSSwapBuffer( Vector2i& mCursor, Vector2i& mCursorWarp )
{
	glBindRenderbuffer( GL_RENDERBUFFER, _ge_ios_colorRenderbuffer );
	[ _ge_eagl_window_context presentRenderbuffer:GL_RENDERBUFFER ];
	
	SInt32 result;
	do {
		result = CFRunLoopRunInMode( kCFRunLoopDefaultMode, 0, TRUE );
	} while( result == kCFRunLoopRunHandledSource );

	mCursor.x = _ge_mouse_x;
	mCursor.y = _ge_mouse_y;
	mCursorWarp.x = 0;
	mCursorWarp.y = 0;
}


std::fstream* _ge_iOSOpen( const char* file, std::ios_base::openmode mode )
{
	std::fstream* ret = nullptr;

	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *docs_dir = [paths objectAtIndex:0];
	NSString* aFile = [docs_dir stringByAppendingPathComponent: [NSString stringWithUTF8String:file]];

	ret = new std::fstream( [aFile fileSystemRepresentation], mode );

	if ( !ret->is_open() ) {
		NSString* path = [[NSBundle mainBundle] pathForResource: [NSString stringWithUTF8String:file] ofType:nil];
		ret = new std::fstream( [path cStringUsingEncoding:1], mode );
	}

	return ret;
}


int main( int ac, char** av )
{
	@autoreleasepool {
		_ge_argc = ac;
		_ge_argv = av;
		return UIApplicationMain( 0, nullptr, nil, NSStringFromClass([AppDelegate class]) );
	}
}


@end
