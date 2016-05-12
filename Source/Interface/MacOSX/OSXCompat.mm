#include <OgreOSXCocoaWindow.h>
#include <OgrePlatform.h>
#include <OgreTimer.h>
#include "OSXCompat.h"

#include "Application.h"

using namespace Ogre;

static Application * app = nullptr;

@implementation AppDelegate

@synthesize mTimer;
@synthesize mMenu;
@dynamic mLastFrameTime;
@dynamic mStartTime;

- (NSTimeInterval)mLastFrameTime
{
	return mLastFrameTime;
}

- (void)setLastFrameTime:(NSTimeInterval)frameInterval
{
	// Frame interval defines how many display frames must pass between each time the
	// display link fires. The display link will only fire 30 times a second when the
	// frame internal is two on a display that refreshes 60 times a second. The default
	// frame interval setting of one will fire 60 times a second when the display refreshes
	// at 60 times a second. A frame interval setting of less than one results in undefined
	// behavior.
	if (frameInterval >= 1)
	{
		mLastFrameTime = frameInterval;
	}
}

- (void)go
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	mLastFrameTime = 1;
	mTimer = nil;

	app = new Application();

	app->init();

	mTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)(1.0f / 60.0f) * mLastFrameTime
											  target:(AppDelegate*)self
											selector:@selector(renderOneFrame:)
											userInfo:nil
											 repeats:YES];
	OSXCocoaWindow * window = static_cast<OSXCocoaWindow*>(app->mRenderWindow);
	NSWindow * nswindow = window->ogreWindow();
	[[nswindow standardWindowButton:NSWindowCloseButton] setEnabled:YES];
	[[nswindow standardWindowButton:NSWindowMiniaturizeButton] setEnabled:YES];

	[NSApp setMenu:mMenu];

	[pool release];
}

- (void)renderOneFrame:(id)sender
{
	if (app && app->mRoot && app->mRunning)
	{
		mStartTime = app->mRoot->getTimer()->getMillisecondsCPU();
		app->mRoot->renderOneFrame();
		mLastFrameTime = app->mRoot->getTimer()->getMillisecondsCPU() - mStartTime;
	}
	else
	{
		[mTimer invalidate];
		mTimer = nil;
		[NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
	}
}

- (void)applicationDidFinishLaunching:(NSNotification *)application {
	mLastFrameTime = 1;
	mTimer = nil;
	
	[self go];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
	return YES;
}

- (void)shutdown {
	if(mDisplayLink)
	{
		CVDisplayLinkStop(mDisplayLink);
		CVDisplayLinkRelease(mDisplayLink);
		mDisplayLink = nil;
	}

	[NSApp terminate:nil];
}
@end
