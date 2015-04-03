//
//  LaputaSimpleGlassesRenderer.m
//  Laputa
//
//  Created by Howard Wang on 15-3-30.
//
//

#import "LaputaScreenRenderer.h"
#import "LaputaSimpleGlassesRenderer.hh"

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

using namespace std;
using namespace glm;

@interface LaputaSimpleGlassesRenderer ()
{
    //it contains a screen renderer
    LaputaScreenRenderer* _screenRenderer;
    
    /* EGL assets */
    EAGLContext *_oglContext; //egl context
    CVOpenGLESTextureCacheRef _renderTextureCache; //destination pixelbuffer texture
    
    /* Opengles assets */
    GLuint _programIDL; //compiled shader program for lens
    GLuint _programIDF; //compiled shader program for frame
    GLint _matrixIDL; //matrix for lens in vertex shader
    GLint _matrixIDF; //matrix for frame in vertex shader
    GLuint _vertexbufferF; //vertex array for frames
    GLuint _vertexbufferL; //vertex array for lens
    glm::mat4 _MVP; //matrix for rotation
    GLuint _offscreenBufferHandle; //offscreen buffer
    
    /*meshes from glasses object file*/
    vector<vec3> _verticesF; //frame
    vector<vec3> _verticesL; //lens
}
@end

enum {
    ATTRIB_VERTEXPOSITION_MODELSPACE, // "vertexPosition_modelspace" in vertext shader
    NUM_ATTRIBUTES
};

enum {
    UNIFORM_MVP, // "MVP" in vertext shader
    NUM_UNIFORMS
};

@implementation LaputaSimpleGlassesRenderer

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
    }
    return self;
}

- (void)dealloc
{
    _screenRenderer = nil;
    [self deleteBuffers];
    [_oglContext release];
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

- (CVPixelBufferRef)copyRenderedPixelBuffer:(CVPixelBufferRef)origPixelBuffer
{
    CVReturn err = noErr;
    CVPixelBufferRef  dstPixelBuffer = [_screenRenderer copyRenderedPixelBuffer:origPixelBuffer];

    if ( _offscreenBufferHandle == 0 ) {
        @throw [NSException exceptionWithName:NSInternalInconsistencyException reason:@"Unintialized buffer" userInfo:nil];
        return NULL;
    }
    
    if ( dstPixelBuffer == nil ) {
        @throw [NSException exceptionWithName:NSInvalidArgumentException reason:@"NULL pixel buffer" userInfo:nil];
        return NULL;
    }
    
    const CMVideoDimensions dstDimensions = { (int32_t)CVPixelBufferGetWidth(dstPixelBuffer), (int32_t)CVPixelBufferGetHeight(dstPixelBuffer) };
    
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
        glViewport( 0, 0, dstDimensions.width, dstDimensions.height );
        
        // Set up our destination pixel buffer as the framebuffer's render target.
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( CVOpenGLESTextureGetTarget( dstTexture ), CVOpenGLESTextureGetName( dstTexture ) );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, CVOpenGLESTextureGetTarget( dstTexture ), CVOpenGLESTextureGetName( dstTexture ), 0 );
        
        glUseProgram( _programIDL );
        
        glUniformMatrix4fv(_matrixIDL, 1, GL_FALSE, &MVP[0][0]);
        
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(ATTRIB_VERTEXPOSITION_MODELSPACE);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexbufferL);
        glVertexAttribPointer(
                              ATTRIB_VERTEXPOSITION_MODELSPACE, // match the layout(vertexPosition_modelspace) in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)_verticesL.size()); // 3 indices starting at 0 -> 1 triangle
        glDisableVertexAttribArray(ATTRIB_VERTEXPOSITION_MODELSPACE);
        
        //////////////////////
        //Draw the frames
        //////////////////////
        //bind to frames
        glUseProgram(_programIDF);
        glUniformMatrix4fv(_matrixIDF, 1, GL_FALSE, &MVP[0][0]);
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(ATTRIB_VERTEXPOSITION_MODELSPACE);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexbufferF);
        glVertexAttribPointer(
                              ATTRIB_VERTEXPOSITION_MODELSPACE, //match the layout(vertexPosition_modelspace) in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)_verticesF.size()); // 3 indices starting at 0 -> 1 triangle
        glDisableVertexAttribArray(ATTRIB_VERTEXPOSITION_MODELSPACE);

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
    
    //cv::Mat texture_cv = cv::imread("/Users/Xavier/CodingProject/3DGlassesRender/3DGlasses/test.bmp");
    
    ////////////////////////
    //first step
    //Load model with ASSIMP
    ////////////////////////
    Assimp::Importer importer;
    NSString *glassesFilePath = [[NSBundle mainBundle] pathForResource:@"SimpleGlasses" ofType:@"obj"];
    const aiScene* scene = NULL;
    if ( glassesFilePath ) {
        scene = importer.ReadFile([glassesFilePath UTF8String], aiProcess_Triangulate);
    }
    if (!scene){
        NSLog( @"Error at Model loading error");
        success = NO;
        [self cleanup:success oldContext:oldContext];
        return success;
    }
    
    aiMesh * meshF = scene->mMeshes[0]; //Frame mesh
    aiMesh * meshL = scene->mMeshes[1]; //Lens mesh
    
    for (int i = 0; i < meshF->mNumFaces; i++){
        const aiFace & face = meshF->mFaces[i];
        
        for (int j = 0; j < 3; j++){
            aiVector3D * pos = &(meshF->mVertices[face.mIndices[j]]);
            vec3 v(pos->x, pos->y, pos->z);
            _verticesF.push_back(v);
        }
    }
    
    for (int i = 0; i < meshL->mNumFaces; i++){
        const aiFace & face = meshL->mFaces[i];
        
        for (int j = 0; j < 3; j++){
            aiVector3D * pos = &(meshL->mVertices[face.mIndices[j]]);
            vec3 v(pos->x, pos->y, pos->z);
            _verticesL.push_back(v);
        }
    }
    
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
    GLint attribLocation[NUM_ATTRIBUTES] = {
        ATTRIB_VERTEXPOSITION_MODELSPACE, //ATTRIB_TEXTUREPOSITON
    };
    GLchar *attribName[NUM_ATTRIBUTES] = {
        (GLchar *)"vertexPosition_modelspace", //"texturecoordinate",
    };
    
    GLint uniformLocation[NUM_UNIFORMS];
    GLchar *uniformName[NUM_UNIFORMS] = {
        (GLchar *)"MVP",
    };
    // Load vertex and fragment shaders for glasses
    const GLchar *vertLSrc = [Tools readFile:@"SimpleVertexShaderL.vertexshader"];
    const GLchar *fragLSrc = [Tools readFile:@"SimpleFragmentShaderL.fragmentshader"];
    
    glueCreateProgram( vertLSrc, fragLSrc,
                      NUM_ATTRIBUTES, (const GLchar **)&attribName[0], attribLocation,
                      NUM_UNIFORMS, (const GLchar **)&uniformName[0], uniformLocation,
                      &_programIDL );
    if ( ! _programIDL ) {
        NSLog( @"Problem initializing the _programIDL." );
        success = NO;
        [self cleanup:success oldContext:oldContext];
        return success;
    }
    _matrixIDL = uniformLocation[UNIFORM_MVP];
    
    const GLchar *vertFSrc = [Tools readFile:@"SimpleVertexShaderF.vertexshader"];
    const GLchar *fragFSrc = [Tools readFile:@"SimpleFragmentShaderF.fragmentshader"];
    glueCreateProgram( vertFSrc, fragFSrc,
                      NUM_ATTRIBUTES, (const GLchar **)&attribName[0], attribLocation,
                      NUM_UNIFORMS, (const GLchar **)&uniformName[0], uniformLocation,
                      &_programIDF );
    if ( ! _programIDF ) {
        NSLog( @"Problem initializing the _programIDF." );
        success = NO;
        [self cleanup:success oldContext:oldContext];
        return success;
    }
    _matrixIDF = uniformLocation[UNIFORM_MVP];
    
    glGenBuffers(1, &_vertexbufferF);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexbufferF);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, _verticesF.size() * sizeof(vec3), &_verticesF[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &_vertexbufferL);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexbufferL);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, _verticesL.size() * sizeof(vec3), &_verticesL[0], GL_STATIC_DRAW);
    
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
    if ( _programIDL ) {
        glDeleteProgram( _programIDL );
        _programIDL = 0;
    }
    if ( _programIDF ) {
        glDeleteProgram( _programIDF );
        _programIDF = 0;
    }
    
    // Cleanup VBO
    if( _vertexbufferF ) {
        glDeleteBuffers(1, &_vertexbufferF);
        _vertexbufferF = 0;
    }
    if( _vertexbufferL ) {
        glDeleteBuffers(1, &_vertexbufferL);
        _vertexbufferL = 0;
    }
    if ( _renderTextureCache ) {
        CFRelease( _renderTextureCache );
        _renderTextureCache = 0;
    }
    if ( oldContext != _oglContext ) {
        [EAGLContext setCurrentContext:oldContext];
    }
}
@end
