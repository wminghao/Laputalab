//
//  material.h
//  Laputa
//
//  Created by Howard Wang on 15-4-6.
//
//

#ifndef __Laputa__material__
#define __Laputa__material__

#include <stdio.h>
#include <string>

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>


struct Vector4f
{
    Vector4f() {
        x = 0;
        y = 0;
        z = 0;
        w = 0;
    }
    Vector4f(float xx,
             float yy,
             float zz,
             float ww){
        x = xx;
        y = yy;
        z = zz;
        w = ww;
    }
    float x;
    float y;
    float z;
    float w;
};

struct Vector3f
{
    Vector3f() {
        x = 0;
        y = 0;
        z = 0;
    }
    Vector3f(float xx,
             float yy,
             float zz){
        x = xx;
        y = yy;
        z = zz;
    }
    float x;
    float y;
    float z;
    
};
struct Vector2f
{
    Vector2f() {
        x = 0;
        y = 0;
    }
    Vector2f(float xx,
             float yy){
        x = xx;
        y = yy;
    }
    float x;
    float y;
};

class Material
{
public:
    Material(GLint texCountLocation,
             GLint diffuseColorLocation,
             GLint textureImageLocation){
        m_texCountLocation = texCountLocation;
        m_diffuseColorLocation = diffuseColorLocation;
        m_textureImageLocation = textureImageLocation;
    }
    virtual ~Material(){}
    virtual bool load() = 0;
    
    virtual void bind(GLenum textureUnit, GLint textureId) = 0;
    virtual void unbind() = 0;
    
protected:
    GLint m_texCountLocation;
    GLint m_diffuseColorLocation;
    GLint m_textureImageLocation;
};
#endif /* defined(__Laputa__material__) */
