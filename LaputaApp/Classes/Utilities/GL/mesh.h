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
#include <Importer.hpp>      // C++ importer interface
#include <scene.h>       // Output data structure
#include <postprocess.h> // Post processing flags

//opengles library
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

//include texture
#include "texture.h"

struct Vector3f
{
    Vector3f() {
        x = 0;
        y = 0;
        z = 0;
    }
    Vector3f(float xx,
             float yy,
             float zz){
        x = xx;
        y = yy;
        z = zz;
    }
    float x;
    float y;
    float z;

};
struct Vector2f
{
    Vector2f() {
        x = 0;
        y = 0;
    }
    Vector2f(float xx,
             float yy){
        x = xx;
        y = yy;
    }
    float x;
    float y;
};

struct Vertex
{
    Vector3f m_pos;
    Vector2f m_tex;
    Vector3f m_normal;

    Vertex() {}

    Vertex(const Vector3f& pos, const Vector2f& tex, const Vector3f& normal)
    {
        m_pos    = pos;
        m_tex    = tex;
        m_normal = normal;
    }
};


class Mesh
{
public:
    Mesh();

    ~Mesh();

    bool LoadMesh(const std::string& Filename);

    void Render();

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
    std::vector<Texture*> m_Textures;
};


#endif	/* MESH_H */

