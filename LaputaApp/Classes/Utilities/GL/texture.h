//
//  texture.h
//  Laputa
//
//  Created by Howard Wang on 15-4-2.
//
//

#ifndef Laputa_texture_h
#define Laputa_texture_h

#include <string>
#include "material.h"

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

class Texture: public Material
{
public:
    Texture(GLint texCountLocation,
            GLint diffuseColorLocation,
            GLint textureImageLocation,
            GLenum TextureTarget, const std::string& FileName);
    
    virtual ~Texture() {}
    bool load();
    
    void bind(GLenum textureUnit, GLint textureId);
    void unbind();
private:
    std::string m_fileName;
    GLenum m_textureTarget;
    GLuint m_textureObj;
};

#endif
