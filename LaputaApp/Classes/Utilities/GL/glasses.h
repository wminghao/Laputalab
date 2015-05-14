//
//  glasses.h
//  Laputa
//
//  Created by Howard Wang on 15-5-13.
//
//

#ifndef Laputa_glasses_h
#define Laputa_glasses_h

#include "ShaderUtilities.h"
#include "matrix.h"

//math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class Mesh;

enum {
    ATTRIB_POSITION, // "position" in vertext shader
    ATTRIB_TEXCOORD, // "TexCoord" in vertext shader
    ATTRIB_NORMAL, // "normal" in vertext shader
    NUM_ATTRIBUTES
};

enum {
    UNIFORM_MVP, // "MVP" in vertext shader
    UNIFORM_WORLD, // "gWorld" in vertext shader
    UNIFORM_VIEWINVERSE, // "viewInverse" in vertext shader
    UNIFORM_NORMALMATRIX, // "NormalMatrix" in vertext shader
    UNIFORM_TEXCOUNT,
    UNIFORM_DIFFUSECOLOR,
    UNIFORM_AMBIENTCOLOR,
    UNIFORM_TEXTUREIMAGE,
    UNIFORM_ENVMAP,
    NUM_UNIFORMS
};

class Glasses{
public:
    Glasses();
    ~Glasses();
    
    bool init(const GLchar *vertLSrc, const GLchar *fragLSrc, const char* glassesFilePath);
    
    bool render(int srcWidth, int srcHeight, GLenum dstTextureTarget, GLuint dstTextureName);
    
    void deinit();
private:
    /*mesh*/
    Mesh* _pMesh;
    
    /* Opengles assets */
    GLuint _programID; //compiled shader program for glasses
    GLint _matrixMVP; //matrix for glasses in vertex shader
    GLint _matrixWorld; //matrix for glasses in vertex shader
    GLint _matrixViewInverse; //matrix for glasses in vertex shader
    GLint _matrixNormalMatrix; //matrix for glasses in vertex shader
    mat4 _Projection; //projection matrix matrix for rotation
    mat4 _World; //world matrix for rotation
    mat4 _View; //view matrix for rotation
    mat3 _NormalMatrix; //normal Matrix matrix
    mat4 _ViewInverse; //view inverse matrix matrix for rotation
    GLuint _offscreenBufferHandle; //offscreen buffer
    GLuint _depthRenderbuffer; //depth render buffer
};

#endif
