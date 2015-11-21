#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#import <GoogleMobileAds/GoogleMobileAds.h>

@interface AppDelegate : UIResponder <UIApplicationDelegate, GLKViewDelegate, GADInterstitialDelegate>

@property(strong, nonatomic) UIWindow *window;
@property(nonatomic, strong) GADInterstitial *interstitial;

- (void)SetAdmobPublisherId:(NSString*)pubid;
- (void)ShowInterstitialAd;
- (float)statusBarHeight;

@end
