#pragma once

#include <AppKit/AppKit.h>

extern id mAppDelegate;

@interface AppDelegate : NSObject<NSApplicationDelegate>
{
	NSTimer * mTimer;
	NSDate * mDate;
	NSTimeInterval mLastFrameTime;
	CVDisplayLinkRef mDisplayLink;
	double mStartTime;
	NSMenu * mMenu;
}

- (void)go;
- (void)renderOneFrame:(id)sender;

@property (retain, atomic) NSTimer *mTimer;
@property (retain, atomic) NSMenu *mMenu;
@property (nonatomic) NSTimeInterval mLastFrameTime;
@property (nonatomic) double mStartTime;

@end
