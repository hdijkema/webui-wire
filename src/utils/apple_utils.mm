
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#include "i_webui_wire.h"

void runOnMainQueueWithoutDeadlocking(void (^block)(void))
{
    if ([NSThread isMainThread])
    {
        block();
    }
    else
    {
        dispatch_async(dispatch_get_main_queue(), block);
    }
}


void process_events_apple()
{
    NSApplication *app = [NSApplication sharedApplication];
    NSEvent *event;

    // Process all pending events
    while ((event = [app nextEventMatchingMask:NSEventMaskAny
                        untilDate:[NSDate distantPast]
                        inMode:NSDefaultRunLoopMode
                        dequeue:YES])) {
        [app sendEvent:event];
    }
}

const char *webwire_command_apple(void *handle, const char *cmd)
{
    __block const char *r = NULL;

    runOnMainQueueWithoutDeadlocking(^{
      r = webwire_command(handle, cmd);
    });

    while(r == NULL) {
      usleep(1000);
    }
    return r;
}

void run_main_app_loop_apple()
{
    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        // Ensure the event loop keeps running
        [app finishLaunching];

        // Run the application event loop
        [app run];
    }
}

void stop_main_app_loop_apple()
{
  @autoreleasepool {
    NSApplication *app = [NSApplication sharedApplication];
    [app terminate:nil];
  }
}

void focus_window_apple(void *c_window)
{
  NSWindow *window = (NSWindow *) c_window;
  //[window makeKeyAndOrderFront:window];
  //[window makeKeyWindow];
  //[NSApp activateIgnoringOtherApps:YES];
  //BOOL a = [window canBecomeKeyWindow];
  //BOOL b = [window canBecomeMainWindow];

  //[window makeMainWindow];
  //[window makeKeyWindow];
  runOnMainQueueWithoutDeadlocking(^{
    [NSApp activateIgnoringOtherApps: YES];
    [window makeMainWindow];
    [window makeKeyWindow];
    [window makeKeyAndOrderFront:window];
    //[window ]
  });

  NSApplication *app = [NSApplication sharedApplication];
  [app run];

  //process_events_apple();
  //[window makeMainWindow];
  //process_events_apple();
  //NSApplication *app = [NSApplication sharedApplication];
  //NSWindow *main_win = [app mainWindow];
  //[window makeKeyAndOrderFront:main_win];
  //[NSApp activateIgnoringOtherApps: YES];
  //[window makeKeyAndOrderFront:nil];
}

void init_app_apple()
{
  [NSApplication sharedApplication];
  [[NSRunningApplication currentApplication] activateWithOptions:(
        NSApplicationActivateAllWindows
  //     // |
  //     // NSApplicationActivateIgnoringOtherApps
    )];
  //[app activateIgnoringOtherApps : YES];
}


