//
//  texture.cpp
//  Laputa
//
//  Created by Howard Wang on 15-4-2.
//
//

#include <stdio.h>
#include "texture.h"

#include <wand/magick_wand.h>
#include <wand/magick-image.h>


Texture::Texture(GLint texCountLocation,
                 GLint diffuseColorLocation,
                 GLint ambientColorLocation,
                 GLint textureImageLocation,
                 const Vector4f& diffuseColor,
                 const Vector4f& ambientColor,
                 GLenum TextureTarget,
                 const std::string& FileName):Material(texCountLocation,
                                                       diffuseColorLocation, ambientColorLocation, textureImageLocation,
                                                       diffuseColor, ambientColor)
{
    m_textureTarget = TextureTarget;
    m_fileName      = FileName;
}

bool Texture::load()
{
    MagickWandGenesis();
    MagickWand *wand = NewMagickWand();
    
    // Read the image - all you need to do is change "logo:" to some other
    // filename to have this resize and, if necessary, convert a different file
    if( MagickReadImage(wand, m_fileName.c_str()) != MagickFalse) {
        // Get the image's width and height
        size_t width,height;
        width = MagickGetImageWidth(wand);
        height = MagickGetImageHeight(wand);
        
        //b/c the input image can be odd width or odd height, mipmap will complain.
        //it's better not to use odd width/height image for performance reasons.
        unsigned char * pixels = (unsigned char*)malloc(sizeof(unsigned char) * width * height * 4);
        if( MagickTrue == MagickGetImagePixels(wand, 0, 0, width, height, "RGBA", CharPixel, pixels)) {
            glGenTextures(1, &m_textureObj);

            glBindTexture(m_textureTarget, m_textureObj);
            glTexImage2D(m_textureTarget, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //no mipmap
            glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //no mipmap
            glBindTexture(m_textureTarget, 0);
            if( width%2 || height%2 ) {
                printf("Warning: width or height not multiple of 2. m_textureObj=%d, width=%ld, height=%ld\n", m_textureObj, width, height);
                glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
                glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            }
        }
        free(pixels);
    } else {
        // Handle the error
        char error_string[256];
        char *description;
        ExceptionType severity;
        
        description=MagickGetException(wand,&severity);
        FormatLocaleString(error_string,250,"%s %s %lu %s\n",GetMagickModule(),description);
        MagickRelinquishMemory(description);
        printf("Warning: cannot read image. error=%s\n", error_string);
    }
    
    MagickWandTerminus();
    
    return true;
}

void Texture::bind(GLenum textureUnit, GLint textureId)
{
    glUniform1i(m_texCountLocation, 1);
    glUniform4f(m_diffuseColorLocation, m_diffuseColor.x, m_diffuseColor.y, m_diffuseColor.z, m_diffuseColor.w);
    glUniform4f(m_ambientColorLocation, m_ambientColor.x, m_ambientColor.y, m_ambientColor.z, m_ambientColor.w);
    glActiveTexture(textureUnit);
    glBindTexture(m_textureTarget, m_textureObj);
    glUniform1i(m_textureImageLocation, textureId); //set the sampler texture to textureId
}
