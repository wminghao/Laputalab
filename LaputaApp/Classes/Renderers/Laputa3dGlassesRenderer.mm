//
//  Laputa3dGlassesRenderer.m
//  Laputa
//
//  Created by Howard Wang on 15-3-30.
//
//

#import "LaputaScreenRenderer.h"
#import "Laputa3dGlassesRenderer.hh"

#import <CoreImage/CoreImage.h>
#import <ImageIO/CGImageProperties.h>

#include <vector>

//glasses
#include "glasses.h"

#include "Tools.h"

//include CVAnalyzer
#import "CVAnalyzerOperation.hh"

#ifdef TAP_TEST
static bool shouldRotate = true;
static bool onTapped = false;
#endif

using namespace std;
using namespace glm;

@interface Laputa3dGlassesRenderer ()
{
    //it contains a screen renderer for filters
    LaputaScreenRenderer* _screenRenderer;
    
    /* EGL assets */
    EAGLContext *_oglContext; //egl context
    CVOpenGLESTextureCacheRef _renderTextureCache; //destination pixelbuffer texture
    
    /* pixelbuffer pool */
    CVPixelBufferPoolRef _bufferPool;
    CFDictionaryRef _bufferPoolAuxAttributes;
    CMFormatDescriptionRef _outputFormatDescription;
    
    /*glasses*/
    Glasses glasses_;
    
    /*core image context*/
    CIContext* _coreImageContext;
    
    /*CVAnalyzer*/
    CVAnalyzerOperation* cvAnalyzer_;
}
@end

@implementation Laputa3dGlassesRenderer

#pragma mark API

- (instancetype)init
{
    self = [super init];
    if ( self )
    {
        _screenRenderer = [[LaputaScreenRenderer alloc] init];
        if ( ! _screenRenderer ) {
            NSLog( @"Problem with _screenRenderer." );
            [self release];
            return nil;
        }
        _oglContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        if ( ! _oglContext ) {
            NSLog( @"Problem with OpenGL context." );
            [self release];
            return nil;
        }
        
        _coreImageContext = [CIContext contextWithEAGLContext:_oglContext];
        if ( ! _coreImageContext ) {
            NSLog( @"Problem with coreimage context." );
            [self release];
            return nil;
        }
        
        cvAnalyzer_ = [[CVAnalyzerOperation alloc]init];
    }
    return self;
}

- (void)dealloc
{
    _screenRenderer = nil;
    [self deleteBuffers];
    [_oglContext release];
    _coreImageContext = nil;
    [super dealloc];
}

#pragma mark LaputaRenderer

- (BOOL)operatesInPlace
{
    return NO;
}

- (FourCharCode)inputPixelFormat
{
    return kCVPixelFormatType_32BGRA;
}

- (void)prepareForInputWithFormatDescription:(CMFormatDescriptionRef)inputFormatDescription outputRetainedBufferCountHint:(size_t)outputRetainedBufferCountHint
{
    [_screenRenderer prepareForInputWithFormatDescription:inputFormatDescription outputRetainedBufferCountHint:outputRetainedBufferCountHint];
    
    // The input and output dimensions are the same. This renderer doesn't do any scaling.
    CMVideoDimensions dimensions = CMVideoFormatDescriptionGetDimensions( inputFormatDescription );
    
    [self deleteBuffers];
    if ( ! [self initializeBuffersWithOutputDimensions:dimensions retainedBufferCountHint:outputRetainedBufferCountHint] ) {
        @throw [NSException exceptionWithName:NSInternalInconsistencyException reason:@"Problem preparing renderer." userInfo:nil];
    }
}

- (void)reset
{
    [_screenRenderer reset];
    [self deleteBuffers];
}

//#define BEAUTIFICATION_ENABLED

- (CVPixelBufferRef)copyRenderedPixelBuffer:(CVPixelBufferRef)origPixelBuffer
{
    CVReturn err = noErr;
    CVPixelBufferRef  dstPixelBuffer = origPixelBuffer;
    [cvAnalyzer_ processImage:dstPixelBuffer andOnTap:onTapped];
    if( onTapped ) {
        printf("-----on Tapped---\r\n");
        onTapped = false;
    }
    
    if ( dstPixelBuffer == nil ) {
        @throw [NSException exceptionWithName:NSInvalidArgumentException reason:@"NULL pixel buffer" userInfo:nil];
        return NULL;
    }
    
    const CMVideoDimensions srcDimensions = { (int32_t)CVPixelBufferGetWidth(dstPixelBuffer), (int32_t)CVPixelBufferGetHeight(dstPixelBuffer) };
    const CMVideoDimensions dstDimensions = CMVideoFormatDescriptionGetDimensions( _outputFormatDescription );
    if ( srcDimensions.width != dstDimensions.width || srcDimensions.height != dstDimensions.height ) {
        @throw [NSException exceptionWithName:NSInvalidArgumentException reason:@"Invalid pixel buffer dimensions" userInfo:nil];
        return NULL;
    }
    
    if ( CVPixelBufferGetPixelFormatType( dstPixelBuffer ) != kCVPixelFormatType_32BGRA ) {
        @throw [NSException exceptionWithName:NSInvalidArgumentException reason:@"Invalid pixel buffer format" userInfo:nil];
        return NULL;
    }
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    if ( oldContext != _oglContext ) {
        if ( ! [EAGLContext setCurrentContext:_oglContext] ) {
            @throw [NSException exceptionWithName:NSInternalInconsistencyException reason:@"Problem with OpenGL context" userInfo:nil];
            return NULL;
        }
    }
    
    CVOpenGLESTextureRef dstTexture = NULL;
    
    //////////////////////////////////////////////////
    //destination texture mapped to output pixelbuffer
    //////////////////////////////////////////////////
    err = CVOpenGLESTextureCacheCreateTextureFromImage( kCFAllocatorDefault,
                                                       _renderTextureCache,
                                                       dstPixelBuffer,
                                                       NULL,
                                                       GL_TEXTURE_2D,
                                                       GL_RGBA,
                                                       dstDimensions.width,
                                                       dstDimensions.height,
                                                       GL_BGRA,
                                                       GL_UNSIGNED_BYTE,
                                                       0,
                                                       &dstTexture );
    
    if ( ! dstTexture || err ) {
        NSLog( @"Error at CVOpenGLESTextureCacheCreateTextureFromImage %d", err );
    } else {
        assert(CVOpenGLESTextureGetTarget( dstTexture ) == GL_TEXTURE_2D);
        if( !glasses_.render( srcDimensions.width, srcDimensions.height, CVOpenGLESTextureGetName( dstTexture ) ) ) {
            NSLog( @"Error at glasses_.Render");
        }
    }
    
bail:
    if ( oldContext != _oglContext ) {
        [EAGLContext setCurrentContext:oldContext];
    }
    if ( dstTexture ) {
        CFRelease( dstTexture );
    }
    //if we apply filter
    //return [_screenRenderer copyRenderedPixelBuffer:dstPixelBuffer];
    return CVPixelBufferRetain( dstPixelBuffer );
}

- (CMFormatDescriptionRef)outputFormatDescription
{
    return _outputFormatDescription;
}

#pragma mark Internal

-(void) cleanup:(BOOL)success oldContext:(EAGLContext *)oldContext
{
    if ( ! success ) {
        [self deleteBuffers];
    }
    if ( oldContext != _oglContext ) {
        [EAGLContext setCurrentContext:oldContext];
    }
}

- (BOOL)initializeBuffersWithOutputDimensions:(CMVideoDimensions)outputDimensions retainedBufferCountHint:(size_t)clientRetainedBufferCountHint
{
    BOOL success = YES;
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    if ( oldContext != _oglContext ) {
        if ( ! [EAGLContext setCurrentContext:_oglContext] ) {
            @throw [NSException exceptionWithName:NSInternalInconsistencyException reason:@"Problem with OpenGL context" userInfo:nil];
            return NO;
        }
    }
    
    /////////////////////////
    //Instantiate glasses model
    /////////////////////////
    const GLchar *vertLSrc = [Tools readFile:@"3dGlassesVertexShader.vsh"];
    const GLchar *fragLSrc = [Tools readFile:@"3dGlassesFragmentShader.fsh"];
    NSString *glassesFilePath = [[NSBundle mainBundle] pathForResource:@"RanGlass" ofType:@"obj"];
    if( !glasses_.init(vertLSrc, fragLSrc, [glassesFilePath UTF8String], 90.0f)) {
        NSLog( @"Problem initializing the _programIDL." );
        success = NO;
        [self cleanup:success oldContext:oldContext];
    }
    
    ///////////////////
    //buffer management
    ///////////////////
    CVReturn err = CVOpenGLESTextureCacheCreate( kCFAllocatorDefault, NULL, _oglContext, NULL, &_renderTextureCache );
    if ( err ) {
        NSLog( @"Error at CVOpenGLESTextureCacheCreate %d", err );
        success = NO;
        [self cleanup:success oldContext:oldContext];
        return success;
    }
    size_t maxRetainedBufferCount = clientRetainedBufferCountHint;
    _bufferPool = createPixelBufferPool( outputDimensions.width, outputDimensions.height, kCVPixelFormatType_32BGRA, (int32_t)maxRetainedBufferCount );
    if ( ! _bufferPool ) {
        NSLog( @"Problem initializing a buffer pool." );
        success = NO;
        [self cleanup:success oldContext:oldContext];
        return success;
    }
    
    _bufferPoolAuxAttributes = createPixelBufferPoolAuxAttributes( (int32_t)maxRetainedBufferCount );
    preallocatePixelBuffersInPool( _bufferPool, _bufferPoolAuxAttributes );
    
    CMFormatDescriptionRef outputFormatDescription = NULL;
    CVPixelBufferRef testPixelBuffer = NULL;
    CVPixelBufferPoolCreatePixelBufferWithAuxAttributes( kCFAllocatorDefault, _bufferPool, _bufferPoolAuxAttributes, &testPixelBuffer );
    if ( ! testPixelBuffer ) {
        NSLog( @"Problem creating a pixel buffer." );
        success = NO;
        [self cleanup:success oldContext:oldContext];
        return success;
    }
    CMVideoFormatDescriptionCreateForImageBuffer( kCFAllocatorDefault, testPixelBuffer, &outputFormatDescription );
    _outputFormatDescription = outputFormatDescription;
    CFRelease( testPixelBuffer );
    
    [self cleanup:success oldContext:oldContext];
    return success;
}

- (void)deleteBuffers
{
    EAGLContext *oldContext = [EAGLContext currentContext];
    if ( oldContext != _oglContext ) {
        if ( ! [EAGLContext setCurrentContext:_oglContext] ) {
            @throw [NSException exceptionWithName:NSInternalInconsistencyException reason:@"Problem with OpenGL context" userInfo:nil];
            return;
        }
    }
    glasses_.deinit();
    if ( _renderTextureCache ) {
        CFRelease( _renderTextureCache );
        _renderTextureCache = 0;
    }
    if ( _bufferPool ) {
        CFRelease( _bufferPool );
        _bufferPool = NULL;
    }
    if ( _bufferPoolAuxAttributes ) {
        CFRelease( _bufferPoolAuxAttributes );
        _bufferPoolAuxAttributes = NULL;
    }
    if ( _outputFormatDescription ) {
        CFRelease( _outputFormatDescription );
        _outputFormatDescription = NULL;
    }
    if ( oldContext != _oglContext ) {
        [EAGLContext setCurrentContext:oldContext];
    }
}

#ifdef TAP_TEST
- (void)onTap
{
    shouldRotate = !shouldRotate;
    onTapped = true;
}
#endif
@end
