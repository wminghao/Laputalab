//
//  color.cpp
//  Laputa
//
//  Created by Howard Wang on 15-4-6.
//
//

#include "color.h"
#include <stdio.h>


Color::Color(GLint texCountLocation,
             GLint diffuseColorLocation,
             GLint textureImageLocation,
             const Vector4f& color):Material(texCountLocation, diffuseColorLocation, textureImageLocation)
{
    m_color = color;
}

bool Color::load()
{
    return true;
}

void Color::bind(GLenum textureUnit, GLint textureId)
{
    glUniform1i(m_texCountLocation, 0);
    glUniform4f(m_diffuseColorLocation, m_color.x, m_color.y, m_color.z, m_color.w);
}
