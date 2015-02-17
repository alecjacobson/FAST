//
//  skinningAppDelegate.m
//  skinning
//
//  Created by Alec Jacobson on 9/14/11.
//  Copyright 2011 New York University. All rights reserved.
//

#import "skinningAppDelegate.h"

@implementation skinningAppDelegate


- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
  return YES;
}
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
  [basic_opengl_view openDocumentFromFileName:filename];
  return YES;
}  
- (void)applicationWillTerminate:(NSNotification *)aNotification
{
  [basic_opengl_view terminate:aNotification];
}
- (NSApplicationTerminateReply) applicationShouldTerminate: (NSApplication*) sender 
{ 
  return NSTerminateNow; 
} 

@end
