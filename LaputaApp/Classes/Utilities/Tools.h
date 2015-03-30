//
//  Tools.h
//  Laputa
//
//  Created by Howard Wang on 15-3-30.
//
//

#import <Foundation/Foundation.h>

@interface Tools : NSObject
+ (const GLchar *)readFile:(NSString *)name;
@end

#if defined __cplusplus
extern "C" {
#endif

CVPixelBufferPoolRef createPixelBufferPool( int32_t width, int32_t height, FourCharCode pixelFormat, int32_t maxBufferCount );
CFDictionaryRef createPixelBufferPoolAuxAttributes( int32_t maxBufferCount );
void preallocatePixelBuffersInPool( CVPixelBufferPoolRef pool, CFDictionaryRef auxAttributes );
    
#if defined __cplusplus
}
#endif
