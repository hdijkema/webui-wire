
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
    NSApplication *app = [NSApplication sharedApplication];
    [app terminate:nil];
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


