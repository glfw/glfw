#import <AppKit/NSApplication.h> // NSApplicationDelegate
#import <AppKit/NSWindow.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (assign, nonatomic) IBOutlet NSWindow *window;

@property (assign, atomic) NSTimer *stepTimer;

-(IBAction)createGlfwWindow:(id)sender;

-(IBAction)singleStep:(id)sender;

-(IBAction)closeGlfwWindow:(id)sender;

-(IBAction)run:(id)sender;

-(IBAction)stop:(id)sender;

@end
