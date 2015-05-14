 //
//  glasses.cpp
//  Laputa
//
//  Created by Howard Wang on 15-5-13.
//
//

#include <stdio.h>
#include "glasses.h"
#include "mesh.h"
#include "ShaderUtilities.h"
#include "err.h"

Glasses::Glasses()
{
    _pMesh = new Mesh();
}

Glasses::~Glasses()
{
    deinit();
    delete(_pMesh);
}

bool Glasses::init(const GLchar *vertLSrc, const GLchar *fragLSrc, const char* glassesFilePath, float zRotateInDegree)
{
    bool ret = false;
    
    /////////////////////
    // offscreen buffer
    /////////////////////
    glEnable(GL_CULL_FACE);  //enable culling to speed up rendering.
    glEnable(GL_DEPTH_TEST); //MUST enable depth buffer
    glEnable(GL_DITHER); //enable dithering.
    glEnable(GL_BLEND); //enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glGenFramebuffers( 1, &_offscreenBufferHandle );
    glBindFramebuffer( GL_FRAMEBUFFER, _offscreenBufferHandle );
    
    glGenRenderbuffers(1, &_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
    
    /////////////////
    // shader program
    /////////////////
    //glasses shaders
    // Load vertex and fragment shaders
    GLint attribLocation[NUM_ATTRIBUTES] = {
        ATTRIB_POSITION, // "position" in vertext shader
        ATTRIB_TEXCOORD, // "TexCoord" in vertext shader
        ATTRIB_NORMAL, // "normal" in vertext shader,
    };
    GLchar *attribName[NUM_ATTRIBUTES] = {
        (GLchar *)"position",
        (GLchar *)"texCoord",
        (GLchar *)"normal",
    };
    GLint uniformLocation[NUM_UNIFORMS];
    GLchar *uniformName[NUM_UNIFORMS] = {
        (GLchar *)"MVP",
        (GLchar *)"World",
        (GLchar *)"ViewInverse",
        //(GLchar *)"NormalMatrix",
        (GLchar *)"texCount",
        (GLchar *)"diffuseColor",
        (GLchar *)"ambientColor",
        (GLchar *)"textureImage",
        (GLchar *)"envMap",
    };
    
    // Load vertex and fragment shaders
    // Load vertex and fragment shaders for glasses    
    glueCreateProgram( vertLSrc, fragLSrc, NULL,
                      NUM_ATTRIBUTES, (const GLchar **)&attribName[0], attribLocation,
                      NUM_UNIFORMS, (const GLchar **)&uniformName[0], uniformLocation,
                      &_programID );
    if ( _programID ) {
        _matrixMVP = uniformLocation[UNIFORM_MVP];
        _matrixWorld = uniformLocation[UNIFORM_WORLD];
        _matrixViewInverse = uniformLocation[UNIFORM_VIEWINVERSE];
        //_matrixNormalMatrix = uniformLocation[UNIFORM_NORMALMATRIX];
        
        _pMesh->setAttrUni(uniformLocation[UNIFORM_TEXCOUNT], uniformLocation[UNIFORM_DIFFUSECOLOR], uniformLocation[UNIFORM_AMBIENTCOLOR],
                           uniformLocation[UNIFORM_TEXTUREIMAGE], uniformLocation[UNIFORM_ENVMAP],
                           attribLocation[ATTRIB_POSITION], attribLocation[ATTRIB_TEXCOORD], attribLocation[ATTRIB_NORMAL]);
        
        
        ////////////////////////
        //Load model with ASSIMP
        ////////////////////////
        _pMesh->LoadMesh(glassesFilePath);
        
        ////////////////////////
        //Set the matrices
        ////////////////////////
        // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        mat4 Projection = perspective(radians(45.0f), 16.0f/9.0f, 0.5f, 100.0f); //for portrait mode, front/back camera, is: 16:9
        // Or, for an ortho camera :
        //mat4 Projection = ortho(-8.0f,8.0f,-4.5f,4.5f,0.0f,100.0f); // In world coordinates, x/y =16/9 ratio, far-near is big enough
        
        // Camera matrix
        mat4 View       = lookAt(vec3(0,0,10), // Camera is at (0, 0, 10), in World Space
                                 vec3(0,0,0), // and looks at the origin
                                 vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                                 );
        // Model matrix : an identity matrix (model will be at the origin)
        float scaleFactor = 9.0/_pMesh->getWidth() * 0.6; //put the object width the same as portaint mode 9:16
        //mat4 Model      = mat4(1.0f);
        mat4 Model_translation = translate(mat4(1.0f), vec3(0,0,0));
        mat4 Model_rotateZ = rotate(mat4(1.0f), radians(zRotateInDegree), vec3(0,0,1)); //rotate z of 90 degree
        mat4 Model_rotateX = rotate(mat4(1.0f), radians(10.0f), vec3(1,0,0)); //rotate x of 10 degree
        mat4 Model_scale = scale(mat4(1.0f), vec3(scaleFactor,scaleFactor,scaleFactor));
        mat4 Model = Model_translation * Model_rotateZ * Model_rotateX * Model_scale;
        
        _ViewInverse = inverse(View); //inverse of the view matrix
        //_NormalMatrix = transpose(inverse(mat3(Model)));
        _World = Model; //world coordinate.
        _View = View;
        
        // Our ModelViewProjection : multiplication of our 3 matrices
        // Remember, matrix multiplication is the other way around
        _Projection = Projection;
        ret = true;
    }
    return ret;
}

void Glasses::deinit()
{
    if ( _offscreenBufferHandle ) {
        glDeleteFramebuffers( 1, &_offscreenBufferHandle );
        _offscreenBufferHandle = 0;
    }
    if ( _depthRenderbuffer ) {
        glDeleteFramebuffers( 1, &_depthRenderbuffer );
        _depthRenderbuffer = 0;
    }
    if ( _programID ) {
        glDeleteProgram( _programID );
        _programID = 0;
    }
}

bool Glasses::render(int srcWidth, int srcHeight, GLuint dstTextureName)
{
    bool ret = false;
    if ( _offscreenBufferHandle != 0 ) {
        
        //TODO below is the test code to do rotation.
        static float angleInDegree = 0.0f;
        static int sign = -1;
        if(angleInDegree >= 90) {
            sign = -1;
        } else if(angleInDegree <= -90) {
            sign = 1;
        }
        angleInDegree += sign;
        
        
        mat4 World = rotate(_World, radians(angleInDegree), vec3(0,1,0)); //matrix for rotation on y axis
        
#ifdef TAP_TEST
        if( !shouldRotate ) {
            World = _World;
        }
#endif
        
        mat4 MVP = _Projection * _View * World;
        
        //////////////////////
        //Draw the lens
        //////////////////////
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //Bind a framebuffer
        glBindFramebuffer( GL_FRAMEBUFFER, _offscreenBufferHandle );
        
        glViewport( 0, 0, srcWidth, srcHeight);
        
        // Set up our destination pixel buffer as the framebuffer's render target.
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, dstTextureName );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstTextureName, 0 );
        glBindTexture( GL_TEXTURE_2D, 0 );
        
        glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, srcWidth, srcHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
        glUseProgram( _programID );
        
        glUniformMatrix4fv(_matrixMVP, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(_matrixWorld, 1, GL_FALSE, &World[0][0]);
        glUniformMatrix4fv(_matrixViewInverse, 1, GL_FALSE, &_ViewInverse[0][0]);
        //glUniformMatrix4fv(_matrixNormalMatrix, 1, GL_FALSE, &_NormalMatrix[0][0]);
        
        getGLErr("render 1");
        
        //render the meshes
        _pMesh->Render();
        
        getGLErr("render 2");

        // Make sure that outstanding GL commands which render to the destination pixel buffer have been submitted.
        // AVAssetWriter, AVSampleBufferDisplayLayer, and GL will block until the rendering is complete when sourcing from this pixel buffer.
        glFlush();

        ret = true;
    }
    return ret;
}