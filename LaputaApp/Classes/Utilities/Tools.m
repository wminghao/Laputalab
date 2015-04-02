//
//  Tools.m
//  Laputa
//
//  Created by Howard Wang on 15-3-30.
//
//

#import "Tools.h"

@implementation Tools

+ (const GLchar *)readFile:(NSString *)name
{
    NSString *path;
    const GLchar *source;
    
    path = [[NSBundle mainBundle] pathForResource:name ofType: nil];
    source = (GLchar *)[[NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil] UTF8String];
    return source;
}

CVPixelBufferPoolRef createPixelBufferPool( int32_t width, int32_t height, FourCharCode pixelFormat, int32_t maxBufferCount )
{
    CVPixelBufferPoolRef outputPool = NULL;
    
    NSDictionary *sourcePixelBufferOptions = @{ (id)kCVPixelBufferPixelFormatTypeKey : @(pixelFormat),
                                                (id)kCVPixelBufferWidthKey : @(width),
                                                (id)kCVPixelBufferHeightKey : @(height),
                                                (id)kCVPixelBufferCGImageCompatibilityKey: [NSNumber numberWithBool:YES],
                                                (id)kCVPixelBufferCGBitmapContextCompatibilityKey: [NSNumber numberWithBool:YES],
                                                (id)kCVPixelFormatOpenGLESCompatibility : @(YES),
                                                @"IOSurfaceOpenGLESFBOCompatibility":[NSNumber numberWithBool:YES],
                                                @"IOSurfaceOpenGLESTextureCompatibility": [NSNumber numberWithBool:YES],
                                                (id)kCVPixelBufferIOSurfacePropertiesKey : @{ /*empty dictionary*/ }};
    
    NSDictionary *pixelBufferPoolOptions = @{ (id)kCVPixelBufferPoolMinimumBufferCountKey : @(maxBufferCount) };
    
    CVPixelBufferPoolCreate( kCFAllocatorDefault, (CFDictionaryRef)pixelBufferPoolOptions, (CFDictionaryRef)sourcePixelBufferOptions, &outputPool );
    
    return outputPool;
}

CFDictionaryRef createPixelBufferPoolAuxAttributes( int32_t maxBufferCount )
{
    // CVPixelBufferPoolCreatePixelBufferWithAuxAttributes() will return kCVReturnWouldExceedAllocationThreshold if we have already vended the max number of buffers
    return CFRetain( @{ (id)kCVPixelBufferPoolAllocationThresholdKey : @(maxBufferCount) } );
}

void preallocatePixelBuffersInPool( CVPixelBufferPoolRef pool, CFDictionaryRef auxAttributes )
{
    // Preallocate buffers in the pool, since this is for real-time display/capture
    NSMutableArray *pixelBuffers = [[NSMutableArray alloc] init];
    while ( 1 )
    {
        CVPixelBufferRef pixelBuffer = NULL;
        OSStatus err = CVPixelBufferPoolCreatePixelBufferWithAuxAttributes( kCFAllocatorDefault, pool, auxAttributes, &pixelBuffer );
        
        if ( err == kCVReturnWouldExceedAllocationThreshold ) {
            break;
        }
        assert( err == noErr );
        
        [pixelBuffers addObject:(id)pixelBuffer];
        CFRelease( pixelBuffer );
    }
    [pixelBuffers release];
}
@end
