//
//  main.m
//  skinning
//
//  Created by Alec Jacobson on 9/14/11.
//  Copyright 2011 New York University. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <igl/dirname.h>

int main(int argc, char *argv[])
{
  // Make directory same as executable
  printf("getcwd(): %s\n", getcwd(NULL, 0));
  printf("argv[0] %s\n", argv[0]);
  printf("cd %s/../..\n", igl::dirname(argv[0]).c_str());
  chdir((igl::dirname(argv[0])+"/../../../").c_str());
  printf("getcwd(): %s\n", getcwd(NULL, 0));
  return NSApplicationMain(argc, (const char **)argv);
}
