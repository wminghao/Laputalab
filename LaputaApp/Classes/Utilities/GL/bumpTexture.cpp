//
//  bumpTexture.cpp
//  VET
//
//  Created by Howard Wang on 15-9-4.
//  Copyright (c) 2015年 Xingze. All rights reserved.
//

#include "bumpTexture.h"
#include "Output.h"
#include <wand/magick_wand.h>
#include <wand/magick-image.h>
#include "err.h"

BumpTexture::BumpTexture(GLint texCountLocation,
                         GLint diffuseColorLocation,
                         GLint ambientColorLocation,
                         GLint specularColorLocation,
                         GLint textureImageLocation,
                         GLint bumpImageLocation,
                         const Vector4f& diffuseColor,
                         const Vector4f& ambientColor,
                         const Vector4f& specularColor,
                         const std::string& textureFile,
                         const std::string& bumpFile
                         ):Texture(texCountLocation,
                                    diffuseColorLocation, ambientColorLocation, specularColorLocation,
                                    textureImageLocation,
                                    diffuseColor, ambientColor, specularColor,
                                    textureFile)
{
    m_bumpFile          = bumpFile;
    m_bumpImageLocation = bumpImageLocation;
}

bool BumpTexture::load()
{
    //first load parent texture
    Texture::load();
    
    //then load the bump texture
    MagickWandGenesis();
    MagickWand *wand = NewMagickWand();
    
    // Read the image - all you need to do is change "logo:" to some other
    // filename to have this resize and, if necessary, convert a different file
    if( MagickReadImage(wand, m_bumpFile.c_str()) != MagickFalse) {
        // Get the image's width and height
        size_t width,height;
        width = MagickGetImageWidth(wand);
        height = MagickGetImageHeight(wand);
        
        //b/c the input image can be odd width or odd height, mipmap will complain.
        //it's better not to use odd width/height image for performance reasons.
        unsigned char * pixels = (unsigned char*)malloc(sizeof(unsigned char) * width * height * 4);
        if( MagickTrue == MagickGetImagePixels(wand, 0, 0, width, height, "RGB", CharPixel, pixels)) {
            glGenTextures(1, &m_bumpObj);
            
            glBindTexture(GL_TEXTURE_2D, m_bumpObj);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)width, (GLsizei)height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //no mipmap
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //no mipmap
            if( width%2 || height%2 ) {
                OUTPUT("Warning: width or height not multiple of 2. m_bumpObj=%d, width=%ld, height=%ld\n", m_bumpObj, width, height);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            }
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        free(pixels);
    } else {
        /*
         // Handle the error
         char error_string[256];
         char *description;
         ExceptionType severity;
         
         description=MagickGetException(wand,&severity);
         FormatLocaleString(error_string,250,"%s %s %lu %s\n",GetMagickModule(),description);
         MagickRelinquishMemory(description);
         OUTPUT("Warning: cannot read image. error=%s\n", error_string);
         */
        OUTPUT("Warning: cannot read image. texture\n");
    }
    DestroyMagickWand(wand);
    MagickWandTerminus();
    
    return true;
}

void BumpTexture::bind(GLint textureId)
{
    glUniform1i(m_texCountLocation, 4);
    glUniform4f(m_diffuseColorLocation, m_diffuseColor.x, m_diffuseColor.y, m_diffuseColor.z, m_diffuseColor.w);
    glUniform4f(m_ambientColorLocation, m_ambientColor.x, m_ambientColor.y, m_ambientColor.z, m_ambientColor.w);
    glUniform4f(m_specularColorLocation, m_specularColor.x, m_specularColor.y, m_specularColor.z, m_specularColor.w);
    glActiveTexture(GL_TEXTURE0 + textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureObj);
    glUniform1i(m_textureImageLocation, textureId); //set the sampler texture to textureId
    
    glActiveTexture(GL_TEXTURE0 + textureId + 1);
    glBindTexture(GL_TEXTURE_2D, m_bumpObj);
    glUniform1i(m_bumpImageLocation, textureId+1); //set the reflection texture to textureId

}
