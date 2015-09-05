//
//  bumpTexture.h
//  VET
//
//  Created by Howard Wang on 15-9-4.
//  Copyright (c) 2015年 Xingze. All rights reserved.
//

#ifndef __VET__bumpTexture__
#define __VET__bumpTexture__

#include <stdio.h>

#include <string>
#include "texture.h"

class BumpTexture: public Texture
{
public:
    BumpTexture(GLint texCountLocation,
                GLint diffuseColorLocation,
                GLint ambientColorLocation,
                GLint specularColorLocation,
                GLint textureImageLocation,
                GLint bumpImageLocation,
                const Vector4f& diffuseColor,
                const Vector4f& ambientColor,
                const Vector4f& specularColor,
                const std::string& textureFile,
                const std::string& bumpFile);
    
    virtual ~BumpTexture() {
        // Delete texture object
        glDeleteTextures ( 1, &m_textureObj );
    }
    bool load();
    
    void bind(GLint textureId);
protected:
    std::string m_bumpFile;
    GLuint m_bumpObj; //object id generated
    GLint m_bumpImageLocation;
};
#endif /* defined(__VET__bumpTexture__) */
