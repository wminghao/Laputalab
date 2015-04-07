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
             GLint ambientColorLocation,
             GLint textureImageLocation,
             const Vector4f& diffuseColor,
             const Vector4f& ambientColor):Material(texCountLocation, diffuseColorLocation, ambientColorLocation, textureImageLocation)
{
    m_diffuseColor = diffuseColor;
    m_ambientColor = ambientColor;
}

bool Color::load()
{
    return true;
}

void Color::bind(GLenum textureUnit, GLint textureId)
{
    glUniform1i(m_texCountLocation, 0);
    glUniform4f(m_diffuseColorLocation, m_diffuseColor.x, m_diffuseColor.y, m_diffuseColor.z, m_diffuseColor.w);
    glUniform4f(m_ambientColorLocation, m_ambientColor.x, m_ambientColor.y, m_ambientColor.z, m_ambientColor.w);
}
