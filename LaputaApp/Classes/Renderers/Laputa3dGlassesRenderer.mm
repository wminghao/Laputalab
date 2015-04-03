//
//  Laputa3dGlassesRenderer.m
//  Laputa
//
//  Created by Howard Wang on 15-3-30.
//
//

#import "LaputaScreenRenderer.h"
#import "Laputa3dGlassesRenderer.hh"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#import "ShaderUtilities.h"
#import "matrix.h"
#import "Tools.h"

#import <CoreImage/CoreImage.h>
#import <ImageIO/CGImageProperties.h>

#include <vector>

//libassimp imports
#include <Importer.hpp>
#include <scene.h>
#include <postprocess.h>

//math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//opencv imports
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"

//mesh reader
#include "mesh.h"

using namespace std;
using namespace glm;

@interface Laputa3dGlassesRenderer ()
{
    //it contains a screen renderer
    LaputaScreenRenderer* _screenRenderer;
    
    /* EGL assets */
    EAGLContext *_oglContext; //egl context
    CVOpenGLESTextureCacheRef _renderTextureCache; //destination pixelbuffer texture
    
    /* pixelbuffer pool */
    CVPixelBufferPoolRef _bufferPool;
    CFDictionaryRef _bufferPoolAuxAttributes;
    CMFormatDescriptionRef _outputFormatDescription;
    
    /* Opengles assets */
    GLuint _programID; //compiled shader program for glasses
    GLint _matrixID; //matrix for glasses in vertex shader
    glm::mat4 _MVP; //matrix for rotation
    GLuint _offscreenBufferHandle; //offscreen buffer
    
    /*meshes from glasses object file*/
    vector<vec3> _verticesF; //frame
    vector<vec3> _verticesL; //lens
    
    /*core image context*/
    CIContext* _coreImageContext;
    
    /*mesh*/
    Mesh* _pMesh;
}
@end

enum {
    ATTRIB_POSITION, // "position" in vertext shader
    ATTRIB_TEXCOORD, // "TexCoord" in vertext shader
    ATTRIB_NORMAL, // "normal" in vertext shader
    NUM_ATTRIBUTES
};

enum {
    UNIFORM_MVP, // "MVP" in vertext shader
    NUM_UNIFORMS
};

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
        
        _pMesh = new Mesh();

    }
    return self;
}

- (void)dealloc
{
    _screenRenderer = nil;
    [self deleteBuffers];
    [_oglContext release];
    _coreImageContext = nil;
    delete(_pMesh);
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
#ifdef BEAUTIFICATION_ENABLED
    //details see https://developer.apple.com/library/ios/documentation/GraphicsImaging/Conceptual/CoreImaging/ci_autoadjustment/ci_autoadjustmentSAVE.html#//apple_ref/doc/uid/TP30001185-CH11-SW1
    CVPixelBufferRef unEnhancedPixelBuffer = [_screenRenderer copyRenderedPixelBuffer:origPixelBuffer];
    CIImage *toBeEnhancedImage = [CIImage imageWithCVPixelBuffer:unEnhancedPixelBuffer];
    NSDictionary *options = nil;
    if([[toBeEnhancedImage properties] valueForKey:(NSString *)kCGImagePropertyOrientation] == nil) {
        options = @{CIDetectorImageOrientation : [NSNumber numberWithInt:1]};
    } else {
        options = @{CIDetectorImageOrientation : [[toBeEnhancedImage properties] valueForKey:(NSString *)kCGImagePropertyOrientation]};
    }
    NSArray *adjustments = [toBeEnhancedImage autoAdjustmentFiltersWithOptions:options];
    for (CIFilter *filter in adjustments) {
        NSString * filterName = [filter name];
        if( [filterName isEqualToString:@"CIFaceBalance"] ) {
            [filter setValue:toBeEnhancedImage forKey:kCIInputImageKey];
            toBeEnhancedImage = filter.outputImage;
            break;
        }
    }
    CVPixelBufferRef dstPixelBuffer = nil;
    err = CVPixelBufferPoolCreatePixelBufferWithAuxAttributes( kCFAllocatorDefault, _bufferPool, _bufferPoolAuxAttributes, &dstPixelBuffer );
    if ( err == kCVReturnWouldExceedAllocationThreshold ) {
        // Flush the texture cache to potentially release the retained buffers and try again to create a pixel buffer
        CVOpenGLESTextureCacheFlush( _renderTextureCache, 0 );
        err = CVPixelBufferPoolCreatePixelBufferWithAuxAttributes( kCFAllocatorDefault, _bufferPool, _bufferPoolAuxAttributes, &dstPixelBuffer );
    }
    if ( err ) {
        if ( err == kCVReturnWouldExceedAllocationThreshold ) {
            NSLog( @"Pool is out of buffers, dropping frame" );
        }
        else {
            NSLog( @"Error at CVPixelBufferPoolCreatePixelBuffer %d", err );
        }
        @throw [NSException exceptionWithName:NSInvalidArgumentException reason:@"Unintialized pixelbuffer" userInfo:nil];
        return NULL;
    }
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGRect rect = [toBeEnhancedImage extent];
    CVPixelBufferLockBaseAddress(dstPixelBuffer, 0);
    [_coreImageContext render:toBeEnhancedImage
              toCVPixelBuffer:dstPixelBuffer
                       bounds:rect
                   colorSpace:colorSpace];
    CVPixelBufferUnlockBaseAddress(dstPixelBuffer, 0);
    CGColorSpaceRelease(colorSpace);
    CFRelease( origPixelBuffer );
#else
    CVPixelBufferRef  dstPixelBuffer = [_screenRenderer copyRenderedPixelBuffer:origPixelBuffer];
#endif
    
    if ( _offscreenBufferHandle == 0 ) {
        @throw [NSException exceptionWithName:NSInternalInconsistencyException reason:@"Unintialized buffer" userInfo:nil];
        return NULL;
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
        
        //TODO below is the test code to do rotation.
        static float angleInDegree = 0.0f;
        static int sign = 1;
        if(angleInDegree >= 180.0) {
            sign = -1;
        } else if(angleInDegree <= -180.0) {
            sign = 1;
        }
        angleInDegree += 0.1*sign;
        
        glm::mat4 MVP = glm::rotate(_MVP, angleInDegree, glm::vec3(0.1,0.1,0.1)); //matrix for rotation
        
        //////////////////////
        //Draw the lens
        //////////////////////
        //bind to lens
        glBindFramebuffer( GL_FRAMEBUFFER, _offscreenBufferHandle );
        glViewport( 0, 0, srcDimensions.width, srcDimensions.height );
        
        // Set up our destination pixel buffer as the framebuffer's render target.
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( CVOpenGLESTextureGetTarget( dstTexture ), CVOpenGLESTextureGetName( dstTexture ) );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, CVOpenGLESTextureGetTarget( dstTexture ), CVOpenGLESTextureGetName( dstTexture ), 0 );
        
        glUseProgram( _programID );
        
        glUniformMatrix4fv(_matrixID, 1, GL_FALSE, &MVP[0][0]);
        
        //render the meshes
        _pMesh->Render();

        glBindTexture( CVOpenGLESTextureGetTarget( dstTexture ), 0 );
        
        // Make sure that outstanding GL commands which render to the destination pixel buffer have been submitted.
        // AVAssetWriter, AVSampleBufferDisplayLayer, and GL will block until the rendering is complete when sourcing from this pixel buffer.
        glFlush();
    }
    
bail:
    if ( oldContext != _oglContext ) {
        [EAGLContext setCurrentContext:oldContext];
    }
    if ( dstTexture ) {
        CFRelease( dstTexture );
    }
    return dstPixelBuffer;
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
    
    ////////////////////////
    //first step
    //Load model with ASSIMP
    ////////////////////////
    Assimp::Importer importer;
    NSString *glassesFilePath = [[NSBundle mainBundle] pathForResource:@"RanGlass" ofType:@"obj"];
    _pMesh->LoadMesh([glassesFilePath UTF8String]);
    
    /////////////////////
    // offscreen buffer
    /////////////////////
    glDisable( GL_DEPTH_TEST );
    
    glGenFramebuffers( 1, &_offscreenBufferHandle );
    glBindFramebuffer( GL_FRAMEBUFFER, _offscreenBufferHandle );
    
    /////////////////
    // shader program
    /////////////////
    //glasses shaders
    
    // Load vertex and fragment shaders
    // Load vertex and fragment shaders for glasses
    const GLchar *vertLSrc = [Tools readFile:@"3dGlassesVertexShader.vsh"];
    const GLchar *fragLSrc = [Tools readFile:@"3dGlassesFragmentShader.fsh"];
    
    glueCreateProgram( vertLSrc, fragLSrc,
                      0, NULL, 0,
                      0, NULL, 0,
                      &_programID );
    if ( ! _programID ) {
        NSLog( @"Problem initializing the _programIDL." );
        success = NO;
        [self cleanup:success oldContext:oldContext];
        return success;
    }
    _matrixID = glueGetUniformLocation(_programID, "MVP");
    
    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    // Or, for an ortho camera :
    //glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates
    
    // Camera matrix
    glm::mat4 View       = glm::lookAt(
                                       glm::vec3(5,6,5), // Camera is at (4,3,3), in World Space
                                       glm::vec3(0,0,0), // and looks at the origin
                                       glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                                       );
    // Model matrix : an identity matrix (model will be at the origin)
    //glm::mat4 Model      = glm::mat4(1.0f);
    mat4 Model_translation = translate(mat4(1.0f), vec3(0,0,0));
    mat4 Model_rotate = rotate(mat4(1.0f), 90.0f, vec3(0,1,0));
    mat4 Model_scale = scale(mat4(1.0f), vec3(0.6,0.6,0.6));
    mat4 Model = Model_translation * Model_rotate * Model_scale;
    
    // Our ModelViewProjection : multiplication of our 3 matrices
    _MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around
    
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
    if ( _offscreenBufferHandle ) {
        glDeleteFramebuffers( 1, &_offscreenBufferHandle );
        _offscreenBufferHandle = 0;
    }
    if ( _programID ) {
        glDeleteProgram( _programID );
        _programID = 0;
    }
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
@end
