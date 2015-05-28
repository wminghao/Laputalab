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

#ifndef MESH_H
#define	MESH_H

#include <map>
#include <vector>

//math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//assimp library
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

//include materials
#include "material.h"

class Mesh
{
public:
    Mesh();

    ~Mesh();
    
    void setAttrUni(GLint texCountLocation,
                    GLint diffuseColorLocation,
                    GLint ambientColorLocation,
                    GLint textureImageLocation,
                    GLint envMapLocation,
                    GLint positionLocation,
                    GLint texCoordLocation,
                    GLint normalLocation) {
        //map to different uniforms and attributes
        m_texCountLocation = texCountLocation;
        m_diffuseColorLocation = diffuseColorLocation;
        m_ambientColorLocation = ambientColorLocation;
        m_textureImageLocation = textureImageLocation;
        m_envMapLocation = envMapLocation;
        m_positionLocation = positionLocation;
        m_texCoordLocation = texCoordLocation;
        m_normalLocation = normalLocation;
    }
    bool LoadMesh(const std::string& Filename);

    void Render();
    
    float getWidth() { return xMax-xMin;}

private:
    bool InitFromScene(const aiScene* pScene, const std::string& Filename);
    void InitMesh(unsigned int Index, const aiMesh* paiMesh);
    bool InitMaterials(const aiScene* pScene, const std::string& Filename);
    void Clear();
    
#define INVALID_OGL_VALUE 0xFFFFFFFF
#define INVALID_MATERIAL 0xFFFFFFFF

    struct MeshEntry {
        MeshEntry();

        ~MeshEntry();

        void Init(const std::vector<Vertex>& Vertices,
                  const std::vector<unsigned int>& Indices);

        GLuint VB;
        GLuint IB;
        unsigned int NumIndices;
        unsigned int MaterialIndex;
    };

    std::vector<MeshEntry> m_Entries;
    std::vector<Material*> m_Materials;
    
    //map to different uniforms and attributes
    GLint m_texCountLocation;
    GLint m_diffuseColorLocation;
    GLint m_ambientColorLocation;
    GLint m_textureImageLocation;
    GLint m_envMapLocation;
    GLint m_positionLocation;
    GLint m_texCoordLocation;
    GLint m_normalLocation;
    
    //calculate Max and Min
    float xMax;
    float yMax;
    float zMax;
    float xMin;
    float yMin;
    float zMin;
    
#ifdef DESKTOP_MAC
    // Vertex Array Objects Identifiers
    GLuint vao;
#endif
};

#endif	/* MESH_H */

