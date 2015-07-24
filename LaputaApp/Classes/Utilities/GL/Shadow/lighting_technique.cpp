/*

	Copyright 2011 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <limits.h>
#include <string.h>

#include "lighting_technique.h"

LightingTechnique::LightingTechnique()
{   
}

bool LightingTechnique::Init()
{
    if (!Technique::Init()) {
        return false;
    }

    //TODO
    if (!AddShader(GL_VERTEX_SHADER, "/Users/howard/AR/LaputaApp/Classes/Utilities/GL/Shadow/lighting.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "/Users/howard/AR/LaputaApp/Classes/Utilities/GL/Shadow/lighting.fs")) {
        return false;
    }

    if (!Finalize()) {
        return false;
    }

    m_Position = GetUniformLocation("Position");
    m_TexCoord = GetUniformLocation("TexCoord");
    if (m_Position == INVALID_UNIFORM_LOCATION ||
        m_TexCoord == INVALID_UNIFORM_LOCATION ) {
        return false;
    }
    
    m_WVPLocation = GetUniformLocation("gWVP");
    m_LightWVPLocation = GetUniformLocation("gLightWVP");
    m_WorldMatrixLocation = GetUniformLocation("gWorld");
    m_samplerLocation = GetUniformLocation("gSampler");
    m_shadowMapLocation = GetUniformLocation("gShadowMap");

    if (m_WVPLocation == INVALID_UNIFORM_LOCATION ||
        m_LightWVPLocation == INVALID_UNIFORM_LOCATION ||
        m_WorldMatrixLocation == INVALID_UNIFORM_LOCATION ||
        m_samplerLocation == INVALID_UNIFORM_LOCATION ||
        m_shadowMapLocation == INVALID_UNIFORM_LOCATION ) {
        return false;
    }
    
    m_pointLightLocation.Color = GetUniformLocation("gPointLight.Base.Color");
    m_pointLightLocation.Position = GetUniformLocation("gPointLight.Position");
    
    if (m_pointLightLocation.Color == INVALID_UNIFORM_LOCATION ||
        m_pointLightLocation.Position == INVALID_UNIFORM_LOCATION ) {
        return false;
    }
    return true;
}


void LightingTechnique::SetWVP(const mat4& WVP)
{
    glUniformMatrix4fv(m_WVPLocation, 1, GL_TRUE, &WVP[0][0]);
}


void LightingTechnique::SetLightWVP(const mat4& LightWVP)
{
    glUniformMatrix4fv(m_LightWVPLocation, 1, GL_TRUE, &LightWVP[0][0]);
}


void LightingTechnique::SetWorldMatrix(const mat4& WorldInverse)
{
    glUniformMatrix4fv(m_WorldMatrixLocation, 1, GL_TRUE, &WorldInverse[0][0]);
}


void LightingTechnique::SetTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_samplerLocation, TextureUnit);
}

void LightingTechnique::SetShadowMapTextureUnit(unsigned int TextureUnit)
{
    glUniform1i(m_shadowMapLocation, TextureUnit);
}

void LightingTechnique::SetPointLight(const PointLight& pLight)
{
    glUniform3f(m_pointLightLocation.Color, pLight.Color.x, pLight.Color.y, pLight.Color.z);
    glUniform3f(m_pointLightLocation.Position, pLight.Position.x, pLight.Position.y, pLight.Position.z);
}

