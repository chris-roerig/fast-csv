#import <Cocoa/Cocoa.h>
#include <gtk/gtk.h>

static GtkApplication *g_app = nullptr;

@interface MacOpenDelegate : NSObject <NSApplicationDelegate>
@end

@implementation MacOpenDelegate

- (BOOL)application:(NSApplication *)sender openFile:(NSString *)filename {
    if (g_app) {
        GFile *file = g_file_new_for_path([filename UTF8String]);
        GFile *files[] = {file};
        g_application_open(G_APPLICATION(g_app), files, 1, "");
        g_object_unref(file);
        return YES;
    }
    return NO;
}

- (void)application:(NSApplication *)sender openFiles:(NSArray<NSString *> *)filenames {
    if (g_app && [filenames count] > 0) {
        GFile *file = g_file_new_for_path([[filenames objectAtIndex:0] UTF8String]);
        GFile *files[] = {file};
        g_application_open(G_APPLICATION(g_app), files, 1, "");
        g_object_unref(file);
    }
}

@end

extern "C" void setup_mac_open_bridge(GtkApplication *app) {
    g_app = app;
    
    MacOpenDelegate *delegate = [[MacOpenDelegate alloc] init];
    [[NSApplication sharedApplication] setDelegate:delegate];
}
