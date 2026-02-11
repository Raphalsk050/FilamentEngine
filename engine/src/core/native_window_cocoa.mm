#include <filament_engine/core/log.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

// Platform-specific helpers for macOS window setup.
// Used by RenderContext when creating SwapChain on macOS with Vulkan or Metal backends.

extern "C" {

void* fe_cocoa_get_native_view(void* nswindow) {
    NSWindow* win = (__bridge NSWindow*)nswindow;
    NSView* view = [win contentView];
    return (__bridge void*)view;
}

void fe_cocoa_prepare_window(void* nswindow) {
    NSWindow* win = (__bridge NSWindow*)nswindow;
    [win setColorSpace:[NSColorSpace sRGBColorSpace]];
}

void* fe_cocoa_setup_metal_layer(void* nativeView) {
    NSView* view = (__bridge NSView*)nativeView;
    [view setWantsLayer:YES];
    CAMetalLayer* metalLayer = [CAMetalLayer layer];
    metalLayer.bounds = view.bounds;
    metalLayer.drawableSize = [view convertSizeToBacking:view.bounds.size];
    metalLayer.contentsScale = view.window.backingScaleFactor;
    metalLayer.opaque = YES;
    [view setLayer:metalLayer];
    return (__bridge void*)metalLayer;
}

void* fe_cocoa_resize_metal_layer(void* nativeView) {
    NSView* view = (__bridge NSView*)nativeView;
    CAMetalLayer* metalLayer = (CAMetalLayer*)view.layer;
    metalLayer.drawableSize = [view convertSizeToBacking:view.bounds.size];
    metalLayer.contentsScale = view.window.backingScaleFactor;
    return (__bridge void*)metalLayer;
}

} // extern "C"
