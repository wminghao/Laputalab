//
//  color.h
//  Laputa
//
//  Created by Howard Wang on 15-4-6.
//
//

#ifndef __Laputa__color__
#define __Laputa__color__

#include <stdio.h>

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include "material.h"

class Color: public Material
{
public:
    Color(GLint texCountLocation,
          GLint diffuseColorLocation,
          GLint ambientColorLocation,
          GLint textureImageLocation,
          const Vector4f& diffuseColor,
          const Vector4f& ambientColor);
    
    virtual ~Color() {}
    bool load();
    
    void bind(GLenum textureUnit, GLint textureId);
private:
    Vector4f m_diffuseColor;
    Vector4f m_ambientColor;
};
#endif /* defined(__Laputa__color__) */
