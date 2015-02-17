//
//  skinningAppDelegate.h
//  skinning
//
//  Created by Alec Jacobson on 9/14/11.
//  Copyright 2011 New York University. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "BasicOpenGLView.h"


@interface skinningAppDelegate : NSObject
{
  IBOutlet BasicOpenGLView * basic_opengl_view;
}
// Set the application to terminate when all windows (there is only one) are
// closed
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:
(NSApplication *)theApplication;
// Allows the application to open a file on launch and when files are dragged
// to the dock icon of the app
- (BOOL)application:(NSApplication *)theApplication openFile:
(NSString *)filename;
// Triggered when application is about to terminate
- (void)applicationWillTerminate:(NSNotification *)aNotification;
@end
