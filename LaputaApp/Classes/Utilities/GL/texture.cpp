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


Texture::Texture(GLenum TextureTarget, const std::string& FileName)
{
    m_textureTarget = TextureTarget;
    m_fileName      = FileName;
}

bool Texture::Load()
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
        
        unsigned char * pixels = (unsigned char*)malloc(sizeof(char) * width * height * 4);
        MagickGetImagePixels(wand, 0, 0, width, height, "RGBA", CharPixel, pixels);
        
        glGenTextures(1, &m_textureObj);
        glBindTexture(m_textureTarget, m_textureObj);
        glTexImage2D(m_textureTarget, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(m_textureTarget, 0);
        
        free(pixels);
    } else {
        // Handle the error
    }
    
    MagickWandTerminus();
    
    return true;
}

void Texture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(m_textureTarget, m_textureObj);
}
