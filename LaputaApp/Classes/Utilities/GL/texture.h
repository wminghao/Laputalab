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
    
    virtual ~Texture() {
        // Delete texture object
        glDeleteTextures ( 1, &m_textureObj );
    }
    bool load();
    
    void bind(GLenum textureUnit, GLint textureId);
private:
    std::string m_fileName;
    GLenum m_textureTarget; //GL_TEXTURE_2D
    GLuint m_textureObj; //object id generated
};

#endif
