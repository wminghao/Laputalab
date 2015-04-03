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

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

class Texture
{
public:
    Texture(GLenum TextureTarget, const std::string& FileName);
    
    bool Load();
    
    void Bind(GLenum TextureUnit);
    
private:
    std::string m_fileName;
    GLenum m_textureTarget;
    GLuint m_textureObj;
};

#endif
