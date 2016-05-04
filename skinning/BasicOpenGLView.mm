
#import "BasicOpenGLView.h"
// For functions like gluErrorString()
#import <OpenGL/glu.h>
#ifdef __APPLE__
#define _MACOSX
#endif
#import "Skinning.h"
#import "ShaderMode.h"

struct Opaque
{
  Skinning skinning;
};

void reportError (char * strError)
{
  // Set up a fancy font/display for error messages
  NSMutableDictionary *attribs = [NSMutableDictionary dictionary];
  [attribs setObject: [NSFont fontWithName: @"Monaco" size: 9.0f]
    forKey: NSFontAttributeName];
  [attribs setObject: [NSColor whiteColor]
    forKey: NSForegroundColorAttributeName];
  // Build the error message string
  NSString * errString = [NSString stringWithFormat:@"Error: %s.", strError];
  // Display to log
  NSLog (@"%@\n", errString);
}

GLenum glReportError (void)
{
  // Get current OpenGL error flag
  GLenum err = glGetError();
  // If there's an error report it
  if (GL_NO_ERROR != err)
  {
    reportError ((char *) gluErrorString (err));
  }
  return err;
}

@implementation BasicOpenGLView


-(IBAction) openDocument: (id) sender
{
  NSOpenPanel *tvarNSOpenPanelObj  = [NSOpenPanel openPanel];
  // Create an array of strings specifying valid extensions and HFS file types.
  NSArray *fileTypes = [NSArray arrayWithObjects:
    @"obj",
    @"OBJ",
    @"off",
    @"OFF",
    @"mesh",
    @"MESH",
    NSFileTypeForHFSTypeCode('TEXT'),
    nil];
  [tvarNSOpenPanelObj setCanChooseDirectories:YES];
  // Create an Open file... dialog
  [tvarNSOpenPanelObj setAllowedFileTypes:fileTypes ];
  NSInteger tvarNSInteger =  [ tvarNSOpenPanelObj runModal ];
  // If the user selected OK then load the file
  if(tvarNSInteger == NSOKButton)
  {
    // Pass on file name to opener helper
    [self openDocumentFromFileName: [[tvarNSOpenPanelObj URL] path] ];
  }
}

- (BOOL)openDocumentFromFileName:(NSString *) file_name
{
    // convert cocoa string to c string
    const char * c_file_name = [file_name UTF8String];
    bool success;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    BOOL isDir;        
    BOOL exists = [fileManager fileExistsAtPath:file_name isDirectory:&isDir];
    if (exists && isDir)
    {
        success = opaque->skinning.load(c_file_name);
    }else {
        success = opaque->skinning.load_mesh_from_file(c_file_name);
    }
  return success;
}

-(IBAction) saveDocumentAs: (id) sender
{
  NSSavePanel *savePanel = [NSSavePanel savePanel];
  [savePanel setTitle:@"Save as (.obj by default)"];
  // TODO: Add a item to this list corresponding to each file type extension
  // this app supports opening
  // Create an array of strings specifying valid extensions and HFS file types.
  NSArray *fileTypes = [NSArray arrayWithObjects:
    @"obj",
    @"OBJ",
    NSFileTypeForHFSTypeCode('TEXT'),
    nil];
  // Only allow these file types
  [savePanel setAllowedFileTypes:fileTypes];
  [savePanel setTreatsFilePackagesAsDirectories:NO];
  // Allow user to save file as he likes
  [savePanel setAllowsOtherFileTypes:YES];
  // Create save as... dialog
  [savePanel setNameFieldStringValue:@""];
  [savePanel setDirectoryURL:[NSURL fileURLWithPath:NSHomeDirectory()]];
  NSInteger user_choice = [savePanel runModal];
  // If user selected OK then save the file
  if(NSOKButton == user_choice)
  {
    // convert cocoa string to c string
    const char * file_name = [[[savePanel URL] path]  UTF8String];
    // TODO: handle saving default file
    NSLog(@"Saving file to %s", file_name);
    opaque->skinning.save_deformed_mesh_to_file(file_name);
  }
}

-(IBAction) openShader: (id) sender
{
  NSOpenPanel *tvarNSOpenPanelObj  = [NSOpenPanel openPanel];
  [tvarNSOpenPanelObj setTitle:@"Open either .vert or .frag of shader pair"];
  // Create an array of strings specifying valid extensions and HFS file types.
  NSArray *fileTypes = [NSArray arrayWithObjects:
    @"frag",
    @"vert",
    @"FRAG",
    @"VERT",
    NSFileTypeForHFSTypeCode('TEXT'),
    nil];
  // Create an Open file... dialog
  [tvarNSOpenPanelObj setAllowedFileTypes:fileTypes ];
  NSInteger tvarNSInteger =  [ tvarNSOpenPanelObj runModal ];
  // If the user selected OK then load the file
  if(tvarNSInteger == NSOKButton)
  {
    // convert cocoa string to c string
    const char * file_name = [[[tvarNSOpenPanelObj URL] path] UTF8String];
    //// Pass on file name to opener helper
    //NSLog(@"openShader(): %s\n",file_name);
    opaque->skinning.load_shader_pair_from_files(LBS,file_name);
  }
}

-(IBAction) openWeights: (id) sender
{
  NSOpenPanel *tvarNSOpenPanelObj  = [NSOpenPanel openPanel];
  [tvarNSOpenPanelObj setTitle:@"Open weights (.dmat)"];
  // Create an array of strings specifying valid extensions and HFS file types.
  NSArray *fileTypes = [NSArray arrayWithObjects:
    @"dmat",
    @"DMAT",
    NSFileTypeForHFSTypeCode('TEXT'),
    nil];
  // Create an Open file... dialog
  [tvarNSOpenPanelObj setAllowedFileTypes:fileTypes ];
  NSInteger tvarNSInteger =  [ tvarNSOpenPanelObj runModal ];
  // If the user selected OK then load the file
  if(tvarNSInteger == NSOKButton)
  {
    // convert cocoa string to c string
    const char * file_name = [[[tvarNSOpenPanelObj URL] path] UTF8String];
    //// Pass on file name to opener helper
    //NSLog(@"openShader(): %s\n",file_name);
    opaque->skinning.load_weights(file_name);
  }
}

-(IBAction) openExtraWeights: (id) sender
{
  NSOpenPanel *tvarNSOpenPanelObj  = [NSOpenPanel openPanel];
  [tvarNSOpenPanelObj setTitle:@"Open extra weights (.dmat)"];
  // Create an array of strings specifying valid extensions and HFS file types.
  NSArray *fileTypes = [NSArray arrayWithObjects:
    @"dmat",
    @"DMAT",
    NSFileTypeForHFSTypeCode('TEXT'),
    nil];
  // Create an Open file... dialog
  [tvarNSOpenPanelObj setAllowedFileTypes:fileTypes ];
  NSInteger tvarNSInteger =  [ tvarNSOpenPanelObj runModal ];
  // If the user selected OK then load the file
  if(tvarNSInteger == NSOKButton)
  {
    // convert cocoa string to c string
    const char * file_name = [[[tvarNSOpenPanelObj URL] path] UTF8String];
    //// Pass on file name to opener helper
    //NSLog(@"openShader(): %s\n",file_name);
    opaque->skinning.load_extra_weights(file_name);
  }
}

-(IBAction) openBoneRoots: (id) sender
{
  NSOpenPanel *tvarNSOpenPanelObj  = [NSOpenPanel openPanel];
  [tvarNSOpenPanelObj setTitle:@"Open bone roots (.bf)"];
  // Create an array of strings specifying valid extensions and HFS file types.
  NSArray *fileTypes = [NSArray arrayWithObjects:
    @"bf",
    @"BF",
    NSFileTypeForHFSTypeCode('TEXT'),
    nil];
  // Create an Open file... dialog
  [tvarNSOpenPanelObj setAllowedFileTypes:fileTypes ];
  NSInteger tvarNSInteger =  [ tvarNSOpenPanelObj runModal ];
  // If the user selected OK then load the file
  if(tvarNSInteger == NSOKButton)
  {
    // convert cocoa string to c string
    const char * file_name = [[[tvarNSOpenPanelObj URL] path] UTF8String];
    //// Pass on file name to opener helper
    //NSLog(@"openShader(): %s\n",file_name);
    opaque->skinning.load_bone_roots(file_name);
  }
}

-(IBAction) openBoneRootsAnimation: (id) sender
{
  NSOpenPanel *tvarNSOpenPanelObj  = [NSOpenPanel openPanel];
  [tvarNSOpenPanelObj setTitle:@"Open bone roots (.bf)"];
  // Create an array of strings specifying valid extensions and HFS file types.
  NSArray *fileTypes = [NSArray arrayWithObjects:
    @"bf",
    @"BF",
    NSFileTypeForHFSTypeCode('TEXT'),
    nil];
  // Create an Open file... dialog
  [tvarNSOpenPanelObj setAllowedFileTypes:fileTypes ];
  NSInteger tvarNSInteger =  [ tvarNSOpenPanelObj runModal ];
  // If the user selected OK then load the file
  if(tvarNSInteger == NSOKButton)
  {
    // convert cocoa string to c string
    const char * file_name = [[[tvarNSOpenPanelObj URL] path] UTF8String];
    //// Pass on file name to opener helper
    //NSLog(@"openShader(): %s\n",file_name);
    opaque->skinning.load_bone_roots_animation(file_name);
  }
}

-(IBAction) openProject: (id) sender
{
  NSOpenPanel *openPanel = [NSOpenPanel openPanel];
  [openPanel setCanChooseDirectories:YES];
  [openPanel setCanCreateDirectories:YES]; // Added by DustinVoss
  [openPanel setPrompt:@"Choose project folder"]; // Should be localized
  [openPanel setCanChooseFiles:NO];
  NSInteger user_choice = [openPanel runModal];
  // If user selected OK then save the file
  if(NSOKButton == user_choice)
  {
    // convert cocoa string to c string
    const char * folder_name = [[[openPanel URL] path] UTF8String];
    opaque->skinning.load(folder_name);
  } 
}

-(IBAction) openBoneRootsAndWeightsFromTGFAndDMAT: (id) sender
{
  NSOpenPanel *tvarNSOpenPanelObj  = [NSOpenPanel openPanel];
  [tvarNSOpenPanelObj setTitle:@"Open skeleton (.tgf)"];
  // Create an array of strings specifying valid extensions and HFS file types.
  NSArray *fileTypes = [NSArray arrayWithObjects:
    @"tgf",
    @"TGF",
    NSFileTypeForHFSTypeCode('TEXT'),
    nil];
  // Create an Open file... dialog
  [tvarNSOpenPanelObj setAllowedFileTypes:fileTypes ];
  NSInteger tvarNSInteger =  [ tvarNSOpenPanelObj runModal ];
  // If the user selected OK then load the file
  if(tvarNSInteger == NSOKButton)
  {
    // convert cocoa string to c string
    const char * tgf_file_name = [[[tvarNSOpenPanelObj URL] path] UTF8String];

    NSOpenPanel *tvarNSOpenPanelObj2  = [NSOpenPanel openPanel];
    [tvarNSOpenPanelObj2 setTitle:@"Open skeleton (.tgf)"];
    // Create an array of strings specifying valid extensions and HFS file types.
    NSArray *fileTypes2 = [NSArray arrayWithObjects:
      @"dmat",
      @"DMAT",
      NSFileTypeForHFSTypeCode('TEXT'),
      nil];
    // Create an Open file... dialog
    [tvarNSOpenPanelObj2 setAllowedFileTypes:fileTypes2 ];
    NSInteger tvarNSInteger2 =  [ tvarNSOpenPanelObj2 runModal ];
    // If the user selected OK then load the file
    if(tvarNSInteger2 == NSOKButton)
    {
      // convert cocoa string to c string
      const char * dmat_file_name = [[[tvarNSOpenPanelObj2 URL] path] UTF8String];

      //// Pass on file name to opener helper
      //NSLog(@"openShader(): %s\n",file_name);
      opaque->skinning.load_tgf_and_dmat_pair(tgf_file_name,dmat_file_name);
    }
  }
}

-(IBAction) openTexture: (id) sender
{
  NSOpenPanel *tvarNSOpenPanelObj  = [NSOpenPanel openPanel];
  [tvarNSOpenPanelObj setTitle:@"Open texture (.tga)"];
  // Create an array of strings specifying valid extensions and HFS file types.
  NSArray *fileTypes = [NSArray arrayWithObjects:
    @"tga",
    @"TGA",
    NSFileTypeForHFSTypeCode('TEXT'),
    nil];
  // Create an Open file... dialog
  [tvarNSOpenPanelObj setAllowedFileTypes:fileTypes ];
  NSInteger tvarNSInteger =  [ tvarNSOpenPanelObj runModal ];
  // If the user selected OK then load the file
  if(tvarNSInteger == NSOKButton)
  {
    // convert cocoa string to c string
    const char * file_name = [[[tvarNSOpenPanelObj URL] path] UTF8String];
    //// Pass on file name to opener helper
    //NSLog(@"openShader(): %s\n",file_name);
    opaque->skinning.load_texture(file_name);
  }
}

-(IBAction) saveBoneRoots: (id) sender
{
  NSSavePanel *savePanel = [NSSavePanel savePanel]; 
  [savePanel setTitle:@"Save Bone Roots as"];
  // TODO: Add a item to this list corresponding to each file type extension
  // this app supports opening
  // Create an array of strings specifying valid extensions and HFS file types.
  NSArray *fileTypes = [NSArray arrayWithObjects:
    @"bf",
    @"BF",
    NSFileTypeForHFSTypeCode('TEXT'),
    nil];
  // Only allow these file types
  [savePanel setAllowedFileTypes:fileTypes]; 
  [savePanel setTreatsFilePackagesAsDirectories:NO]; 
  // Allow user to save file as he likes
  [savePanel setAllowsOtherFileTypes:YES];
  // Create save as... dialog
  [savePanel setNameFieldStringValue:@""];
  [savePanel setDirectoryURL:[NSURL fileURLWithPath:NSHomeDirectory()]];
  NSInteger user_choice = [savePanel runModal];
  // If user selected OK then save the file
  if(NSOKButton == user_choice)
  {
    // convert cocoa string to c string
    const char * file_name = [[[savePanel URL] path] UTF8String];
    opaque->skinning.save_bone_roots(file_name);
  } 
}

-(IBAction) saveExtraWeights: (id) sender
{
  NSSavePanel *savePanel = [NSSavePanel savePanel]; 
  [savePanel setTitle:@"Save Extra Weights as"];
  // TODO: Add a item to this list corresponding to each file type extension
  // this app supports opening
  // Create an array of strings specifying valid extensions and HFS file types.
  NSArray *fileTypes = [NSArray arrayWithObjects:
    @"dmat",
    @"DMAT",
    NSFileTypeForHFSTypeCode('TEXT'),
    nil];
  // Only allow these file types
  [savePanel setAllowedFileTypes:fileTypes]; 
  [savePanel setTreatsFilePackagesAsDirectories:NO]; 
  // Allow user to save file as he likes
  [savePanel setAllowsOtherFileTypes:YES];
  // Create save as... dialog
  [savePanel setNameFieldStringValue:@""];
  [savePanel setDirectoryURL:[NSURL fileURLWithPath:NSHomeDirectory()]];
  NSInteger user_choice = [savePanel runModal];
  // If user selected OK then save the file
  if(NSOKButton == user_choice)
  {
    // convert cocoa string to c string
    const char * file_name = [[[savePanel URL] path] UTF8String];
    opaque->skinning.save_extra_weights(file_name);
  } 
}

-(IBAction) saveProject: (id) sender
{
  NSOpenPanel *openPanel = [NSOpenPanel openPanel];
  [openPanel setCanChooseDirectories:YES];
  [openPanel setCanCreateDirectories:YES]; // Added by DustinVoss
  [openPanel setPrompt:@"Choose project folder"]; // Should be localized
  [openPanel setCanChooseFiles:NO];
  NSInteger user_choice = [openPanel runModal];
  // If user selected OK then save the file
  if(NSOKButton == user_choice)
  {
    // convert cocoa string to c string
    const char * folder_name = [[[openPanel URL] path] UTF8String];
    opaque->skinning.save(folder_name);
  } 
}

-(IBAction) saveBoneRootsAnimation: (id) sender
{
  NSSavePanel *savePanel = [NSSavePanel savePanel]; 
  [savePanel setTitle:@"Save Bone Roots as"];
  // TODO: Add a item to this list corresponding to each file type extension
  // this app supports opening
  // Create an array of strings specifying valid extensions and HFS file types.
  NSArray *fileTypes = [NSArray arrayWithObjects:
    @"bf",
    @"BF",
    NSFileTypeForHFSTypeCode('TEXT'),
    nil];
  // Only allow these file types
  [savePanel setAllowedFileTypes:fileTypes]; 
  [savePanel setTreatsFilePackagesAsDirectories:NO]; 
  // Allow user to save file as he likes
  [savePanel setAllowsOtherFileTypes:YES];
  // Create save as... dialog
  [savePanel setNameFieldStringValue:@""];
  [savePanel setDirectoryURL:[NSURL fileURLWithPath:NSHomeDirectory()]];
  NSInteger user_choice = [savePanel runModal];
  // If user selected OK then save the file
  if(NSOKButton == user_choice)
  {
    // convert cocoa string to c string
    const char * file_name = [[[savePanel URL] path] UTF8String];
    opaque->skinning.save_bone_roots_animation(file_name);
  } 
}

-(void)keyDown:(NSEvent *)theEvent
{
  // NOTE: holding a key on the keyboard starts to signal multiple down
  // events (the only one final up event)
  NSString *characters = [theEvent characters];
  if ([characters length])
  {
    NSPoint location = 
      [self convertPoint:[theEvent locationInWindow] fromView:nil];
    // convert characters to single char
    char character = [characters characterAtIndex:0];
    bool shift_down  = [theEvent modifierFlags] & NSShiftKeyMask;
    bool command_down  = [theEvent modifierFlags] & NSCommandKeyMask;
    bool option_down  = [theEvent modifierFlags] & NSAlternateKeyMask;
    opaque->skinning.key_down(character,location.x,location.y,shift_down,command_down,option_down);
  }
  damage = true;
}

-(void)keyUp:(NSEvent *)theEvent
{
  NSString *characters = [theEvent characters];
  if ([characters length])
  {
    NSPoint location = 
      [self convertPoint:[theEvent locationInWindow] fromView:nil];
    // convert characters to single char
    char character = [characters characterAtIndex:0];
    bool shift_down  = [theEvent modifierFlags] & NSShiftKeyMask;
    bool command_down  = [theEvent modifierFlags] & NSCommandKeyMask;
    bool option_down  = [theEvent modifierFlags] & NSAlternateKeyMask;
    opaque->skinning.key_up(character,location.x,location.y,shift_down,command_down,option_down);
  }
  damage = true;
}

- (void)mouseDown:(NSEvent *)theEvent
{
  // Get location of the click
  NSPoint location = 
    [self convertPoint:[theEvent locationInWindow] fromView:nil];
  bool shift_down  = [theEvent modifierFlags] & NSShiftKeyMask;
  bool command_down  = [theEvent modifierFlags] & NSCommandKeyMask;
  bool option_down  = [theEvent modifierFlags] & NSAlternateKeyMask;
  control_down_on_down  = [theEvent modifierFlags] & NSControlKeyMask;
  if(control_down_on_down)
  {
    opaque->skinning.right_mouse_down(location.x,location.y,shift_down,command_down,option_down);
  }else{
    opaque->skinning.mouse_down(location.x,location.y,shift_down,command_down,option_down);
  }
  damage = true;
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
  // Get location of the click
  NSPoint location = 
    [self convertPoint:[theEvent locationInWindow] fromView:nil];
  bool shift_down  = [theEvent modifierFlags] & NSShiftKeyMask;
  bool command_down  = [theEvent modifierFlags] & NSCommandKeyMask;
  bool option_down  = [theEvent modifierFlags] & NSAlternateKeyMask;
  opaque->skinning.right_mouse_down(location.x,location.y,shift_down,command_down,option_down);
}

- (void)otherMouseDown:(NSEvent *)theEvent
{
  // TODO: Handle other strange mouse button bown events
  // For now just treat as left mouse button down event
  [self mouseDown: theEvent];
}

- (void)mouseUp:(NSEvent *)theEvent
{
  // Get location of the click
  NSPoint location = 
    [self convertPoint:[theEvent locationInWindow] fromView:nil];
  bool shift_down  = [theEvent modifierFlags] & NSShiftKeyMask;
  bool command_down  = [theEvent modifierFlags] & NSCommandKeyMask;
  bool option_down  = [theEvent modifierFlags] & NSAlternateKeyMask;
  if(control_down_on_down)
  {
    opaque->skinning.right_mouse_up(location.x,location.y,shift_down,command_down,option_down);
  }else
  {
    opaque->skinning.mouse_up(location.x,location.y,shift_down,command_down,option_down);
  }
  damage = true;
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
  // Get location of the click
  NSPoint location = 
    [self convertPoint:[theEvent locationInWindow] fromView:nil];
  bool shift_down  = [theEvent modifierFlags] & NSShiftKeyMask;
  bool command_down  = [theEvent modifierFlags] & NSCommandKeyMask;
  bool option_down  = [theEvent modifierFlags] & NSAlternateKeyMask;
  opaque->skinning.right_mouse_up(location.x,location.y,shift_down,command_down,option_down);
}

- (void)otherMouseUp:(NSEvent *)theEvent
{
  // TODO: Handle other strange mouse button up events
  // For now just treat as left mouse button up event
  [self mouseUp: theEvent];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
  NSPoint location = 
    [self convertPoint:[theEvent locationInWindow] fromView:nil];
  bool shift_down  = [theEvent modifierFlags] & NSShiftKeyMask;
  bool command_down  = [theEvent modifierFlags] & NSCommandKeyMask;
  bool option_down  = [theEvent modifierFlags] & NSAlternateKeyMask;
  opaque->skinning.mouse_move(location.x,location.y,shift_down,command_down,option_down);
  damage = true;
}

- (void)mouseDragged:(NSEvent *)theEvent
{
  NSPoint location = 
    [self convertPoint:[theEvent locationInWindow] fromView:nil];
  bool shift_down  = [theEvent modifierFlags] & NSShiftKeyMask;
  bool command_down  = [theEvent modifierFlags] & NSCommandKeyMask;
  bool option_down  = [theEvent modifierFlags] & NSAlternateKeyMask;
  if(control_down_on_down)
  {
    opaque->skinning.right_mouse_drag(location.x,location.y,shift_down,command_down,option_down);
  }else
  {
    opaque->skinning.mouse_drag(location.x,location.y,shift_down,command_down,option_down);
  }
  damage = true;
}

- (void)rightMouseDragged:(NSEvent *)theEvent
{
  NSPoint location = 
    [self convertPoint:[theEvent locationInWindow] fromView:nil];
  bool shift_down  = [theEvent modifierFlags] & NSShiftKeyMask;
  bool command_down  = [theEvent modifierFlags] & NSCommandKeyMask;
  bool option_down  = [theEvent modifierFlags] & NSAlternateKeyMask;
  opaque->skinning.right_mouse_drag(location.x,location.y,shift_down,command_down,option_down);
}

- (void)otherMouseDragged:(NSEvent *)theEvent
{
  // TODO: Handle other strange mouse button drag event
  // For now just treat as left mouse button drag event
  [self mouseDragged: theEvent];
}

- (void)scrollWheel:(NSEvent *)theEvent
{
  NSPoint location = 
    [self convertPoint:[theEvent locationInWindow] fromView:nil];
  opaque->skinning.mouse_scroll(
    location.x,location.y,[theEvent deltaX],[theEvent deltaY]);
  damage = true;
}

- (void) viewDidMoveToWindow
{
  // Listen to all mouse move events (not just dragging)
  [[self window] setAcceptsMouseMovedEvents:YES];
  // When view changes to this window then be sure that we start responding
  // to mouse events
  [[self window] makeFirstResponder:self];
}

- (NSPoint) flip_y:(NSPoint) location
{
  // Get openGL context size
  NSRect rectView = [self bounds];
  // Cocoa gives opposite of OpenGL y direction, flip y direction
  location.y = rectView.size.height - location.y;
  return location;
}

- (void) reshape
{
  NSRect rectView = [self bounds];
  if(NULL != opaque)
  {
    opaque->skinning.resize(rectView.size.width,rectView.size.height);
    opaque->skinning.display();
  }
}

- (void) drawRect:(NSRect)rect
{
  // TODO: handle draw event
  // For now just clear the screen with a time dependent color
  //glClearColor(
  //  fabs(sin([self getElapsedTime])),
  //  fabs(sin([self getElapsedTime]/3)),
  //  fabs(sin([self getElapsedTime]/7)),
  //  0);
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // Elapsed time in seconds: getElapsedTime()
  // Report any OpenGL errors
  glReportError ();
  // Flush all OpenGL calls
  glFlush();
  // Flush OpenGL context
  [[self openGLContext] flushBuffer];
}

- (void) prepareOpenGL
{
  const GLint swapInt = 1;
  // set to vbl sync
  [[self openGLContext] setValues:&swapInt
    forParameter:NSOpenGLCPSwapInterval];
  if(!openGL_initialized)
  {
    // Get command line arguments and find whether stealFocus is set to YES
    NSUserDefaults *args = [NSUserDefaults standardUserDefaults];
    // also find out if app should steal focus
    bool stealFocus = [args boolForKey:@"stealFocus"];
    if(stealFocus)
    {
      // Steal focus means that the apps window will appear in front of all
      // other programs when it launches even in front of the calling
      // application (e.g. a terminal)
      [[NSApplication sharedApplication] activateIgnoringOtherApps:YES];
    }
    // the OpenGL context is initialized (load textures, shaders, etc.)
    openGL_initialized = true;
  }
}

- (void) update
{
  [super update];
}

- (void)animationTimer:(NSTimer *)timer
{
  // TODO: handle timer based redraw (animation) here
  bool your_app_says_to_redraw = true;
  if(your_app_says_to_redraw || damage)
  {
    //damage = false;
    opaque->skinning.display();
//    [self drawRect:[self bounds]];
  }
}

- (void) setStartTime
{
  start_time = CFAbsoluteTimeGetCurrent ();
}

- (CFAbsoluteTime) getElapsedTime
{
  return CFAbsoluteTimeGetCurrent () - start_time;
}

- (BOOL)acceptsFirstResponder
{
  return YES;
}

- (BOOL)becomeFirstResponder
{
  return  YES;
}

- (BOOL)resignFirstResponder
{
  return YES;
}

- (void) awakeFromNib
{
  opaque = new Opaque();
  openGL_initialized = false;
  // keep track of start/launch time
  [self setStartTime];
  // start animation timer
  timer = [NSTimer timerWithTimeInterval:(1.0f/60.0f) target:self
    selector:@selector(animationTimer:) userInfo:nil repeats:YES];
  [[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
  // ensure timer fires during resize
  [[NSRunLoop currentRunLoop] addTimer:timer
  forMode:NSEventTrackingRunLoopMode];
}

- (void) terminate:(NSNotification *)aNotification
{
  delete opaque;
}

@end
