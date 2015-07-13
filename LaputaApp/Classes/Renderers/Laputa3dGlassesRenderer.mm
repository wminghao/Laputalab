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
#include "err.h"

#include "Tools.h"

//include CVAnalyzer
#import "CVAnalyzerOperation.hh"

#ifdef TAP_TEST
static bool shouldRotate = true;
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
    GLuint _candide3Texture;
    
    /* pixelbuffer pool */
    CVPixelBufferPoolRef _bufferPool;
    CFDictionaryRef _bufferPoolAuxAttributes;
    CMFormatDescriptionRef _outputFormatDescription;
    
    /*glasses*/
    Glasses* glasses_;
    mat4 _initMat;
    
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
        
        //cvAnalyzer_ = [[CVAnalyzerOperation alloc]init];
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
    //if we apply filter
    //CVPixelBufferRef  dstPixelBuffer = [_screenRenderer copyRenderedPixelBuffer:origPixelBuffer];;
    CVPixelBufferRef  dstPixelBuffer = origPixelBuffer;
    
    //[cvAnalyzer_ processImage:dstPixelBuffer];
    
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
    
    ////////////////////////////////////////////////////////////
    //for OpenGL-ES, a texture can ONLY be used for both read and write
    //thus, need to copy out the original texture for mapping to candide3
    ////////////////////////////////////////////////////////////
    // Lock the base address of the pixel buffer
    CVPixelBufferLockBaseAddress(origPixelBuffer, 0);
    // Get the number of bytes per row for the pixel buffer
    void *baseAddress = CVPixelBufferGetBaseAddress(origPixelBuffer);
    int w = (int)CVPixelBufferGetWidth( origPixelBuffer );
    int h = (int)CVPixelBufferGetHeight( origPixelBuffer );
    glBindTexture(GL_TEXTURE_2D, _candide3Texture);
    
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                 0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                 GL_RGBA,           // Internal colour format to convert to
                 w,
                 h,
                 0,                 // Border width in pixels (can either be 1 or 0)
                 GL_BGRA,            // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                 GL_UNSIGNED_BYTE,  // Image data type
                 baseAddress); // The actual image data itself
    glBindTexture(GL_TEXTURE_2D, 0);
    CVPixelBufferUnlockBaseAddress(origPixelBuffer, 0);
    
    //////////////////////////////////////////////////
    //destination texture mapped to output pixelbuffer
    //////////////////////////////////////////////////
    CVOpenGLESTextureRef dstTexture = NULL;
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
        if( !glasses_->render( CVOpenGLESTextureGetName( dstTexture ), _candide3Texture, shouldRotate ) ) {
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
    //if we apply filter [_screenRenderer copyRenderedPixelBuffer:dstPixelBuffer];
    //then return the following:
    //return dstPixelBuffer;
    //else return the following:
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
    glasses_ = new Glasses( outputDimensions.width, outputDimensions.height, false ); //TODO for now disable AA
    NSString *vertLSrcPath = [[NSBundle mainBundle] pathForResource:@"3dGlassesVertexShader" ofType:@"vsh"];
    NSString *fragLSrcPath = [[NSBundle mainBundle] pathForResource:@"3dGlassesFragmentShader" ofType:@"fsh"];
    NSString *glassesFilePath = [[NSBundle mainBundle] pathForResource:@"RanGlass" ofType:@"obj"];
    NSString *candide3FacePath = [[NSBundle mainBundle] pathForResource:@"facelist_184" ofType:@"wfm"];
    NSString *candide3VerticesPath = [[NSBundle mainBundle] pathForResource:@"vertexlist_113" ofType:@"wfm"];
    if( !glasses_->init([vertLSrcPath UTF8String], [fragLSrcPath UTF8String], NULL,
                        [glassesFilePath UTF8String], [candide3FacePath UTF8String], [candide3VerticesPath UTF8String],
                        90.0f, ASPECT_RATIO_16_9,
                        NULL, NULL)) {
        NSLog( @"Problem initializing the _programIDL." );
        success = NO;
        [self cleanup:success oldContext:oldContext];
    }
    
    glGenTextures(1, &_candide3Texture);
    getGLErr("1");
    
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
    delete(glasses_);
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
}
#endif
@end
