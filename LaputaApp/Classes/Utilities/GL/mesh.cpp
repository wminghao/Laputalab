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

#include <assert.h>

#include "mesh.h"

//include texture
#include "texture.h"
#include "reflectionTexture.h"
#include "color.h"

Mesh::MeshEntry::MeshEntry()
{
    VB = INVALID_OGL_VALUE;
    IB = INVALID_OGL_VALUE;
    NumIndices  = 0;
    MaterialIndex = INVALID_MATERIAL;
};

Mesh::MeshEntry::~MeshEntry()
{
    if (VB != INVALID_OGL_VALUE)
    {
        glDeleteBuffers(1, &VB);
    }

    if (IB != INVALID_OGL_VALUE)
    {
        glDeleteBuffers(1, &IB);
    }
}

void Mesh::MeshEntry::Init(const std::vector<Vertex>& Vertices,
                          const std::vector<unsigned int>& Indices)
{
    NumIndices = (unsigned int)Indices.size();

    glGenBuffers(1, &VB);
  	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &IB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * NumIndices, &Indices[0], GL_STATIC_DRAW);
}

Mesh::Mesh()
{
}


Mesh::~Mesh()
{
    Clear();
}


void Mesh::Clear()
{
    for (unsigned int i = 0 ; i < m_Materials.size() ; i++) {
        delete (m_Materials[i]);
    }
}


bool Mesh::LoadMesh(const std::string& Filename)
{
    // Release the previously loaded mesh (if it exists)
    Clear();
    
    bool Ret = false;
    Assimp::Importer Importer;

    const aiScene* pScene = Importer.ReadFile(Filename.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
    if (pScene) {
        Ret = InitFromScene(pScene, Filename);
    } else {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
    }

    return Ret;
}

bool Mesh::InitFromScene(const aiScene* pScene, const std::string& Filename)
{  
    m_Entries.resize(pScene->mNumMeshes);
    m_Materials.resize(pScene->mNumMaterials);

    // Initialize the meshes in the scene one by one
    for (unsigned int i = 0 ; i < m_Entries.size() ; i++) {
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitMesh(i, paiMesh);
    }

    return InitMaterials(pScene, Filename);
}

void Mesh::InitMesh(unsigned int Index, const aiMesh* paiMesh)
{
    m_Entries[Index].MaterialIndex = paiMesh->mMaterialIndex;
    
    std::vector<Vertex> Vertices;
    std::vector<unsigned int> Indices;

    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
    
    printf("Mesh Index=%d, Material Index='%d', vertices=%d, mNumFaces=%d\n", Index,
           paiMesh->mMaterialIndex, paiMesh->mNumVertices, paiMesh->mNumFaces);

    for (unsigned int i = 0 ; i < paiMesh->mNumVertices ; i++) {
        const aiVector3D* pPos      = &(paiMesh->mVertices[i]);
        const aiVector3D* pNormal   = &(paiMesh->mNormals[i]);
        const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

        Vertex v(Vector3f(pPos->x, pPos->y, pPos->z),
                 Vector2f(pTexCoord->x, pTexCoord->y),
                 Vector3f(pNormal->x, pNormal->y, pNormal->z));

        Vertices.push_back(v);
    }

    for (unsigned int i = 0 ; i < paiMesh->mNumFaces ; i++) {
        const aiFace& Face = paiMesh->mFaces[i];
        assert(Face.mNumIndices == 3);
        Indices.push_back(Face.mIndices[0]);
        Indices.push_back(Face.mIndices[1]);
        Indices.push_back(Face.mIndices[2]);
    }

    m_Entries[Index].Init(Vertices, Indices);
}

bool Mesh::InitMaterials(const aiScene* pScene, const std::string& Filename)
{
    // Extract the directory part from the file name
    std::string::size_type SlashIndex = Filename.find_last_of("/");
    std::string Dir;

    if (SlashIndex == std::string::npos) {
        Dir = ".";
    }
    else if (SlashIndex == 0) {
        Dir = "/";
    }
    else {
        Dir = Filename.substr(0, SlashIndex);
    }

    bool Ret = true;

    // Initialize the materials
    for (unsigned int i = 0 ; i < pScene->mNumMaterials ; i++) {
        const aiMaterial* pMaterial = pScene->mMaterials[i];

        m_Materials[i] = NULL;
        
        aiString name;
        pMaterial->Get(AI_MATKEY_NAME,name);
        
        aiColor4D diffuse;
        aiColor4D ambient;
        aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
        aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_AMBIENT, &ambient);
        /*
         All other colors are read as default values.
        aiColor4D specular;
        aiColor4D emissive;
        aiColor4D transparent;
        float shininess = 0.0;
        unsigned int max;
        aiGetMaterialFloatArray(pMaterial, AI_MATKEY_SHININESS, &shininess, &max);
        aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_SPECULAR, &specular);
        aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_EMISSIVE, &emissive);
        aiGetMaterialColor(pMaterial, AI_MATKEY_COLOR_TRANSPARENT, &transparent);
        */
        
        Vector4f diffuseColor(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
        Vector4f ambientColor(ambient.r, ambient.g, ambient.b, ambient.a);
        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString Path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                std::string FullPath = Dir + "/" + Path.data;
                if( !strcmp(Path.data, "ramp1-nurbsToPoly1.png") ) {
                    std::string reflectionFullPath = Dir + "/Brooklyn_Bridge_Planks_1k.hdr";
                    m_Materials[i] = new ReflectionTexture(m_texCountLocation,
                                                           m_diffuseColorLocation,
                                                           m_ambientColorLocation,
                                                           m_textureImageLocation,
                                                           diffuseColor,
                                                           ambientColor,
                                                           FullPath.c_str(),
                                                           m_envMapLocation,
                                                           reflectionFullPath.c_str()
                                                           );
                } else {
                    m_Materials[i] = new Texture(m_texCountLocation,
                                             m_diffuseColorLocation,
                                             m_ambientColorLocation,
                                             m_textureImageLocation,
                                             diffuseColor,
                                             ambientColor,
                                             FullPath.c_str());
                }

                if (!m_Materials[i]->load()) {
                    printf("Error loading texture '%s'\n", FullPath.c_str());
                    delete m_Materials[i];
                    m_Materials[i] = NULL;
                    Ret = false;
                } else {
                    printf("Loaded texture index:%d, name %s file: %s coord:%.2f, %.2f, %.2f, %.2f: %.2f, %.2f, %.2f, %.2f\n",
                           i, name.C_Str(), Path.data,
                           diffuseColor.x, diffuseColor.y, diffuseColor.z, diffuseColor.w,
                           ambientColor.x, ambientColor.y, ambientColor.z, ambientColor.w);
                }
            }
        } else {
            m_Materials[i] = new Color(m_texCountLocation,
                                       m_diffuseColorLocation,
                                       m_ambientColorLocation,
                                       m_textureImageLocation,
                                       diffuseColor,
                                       ambientColor);
            printf("Loaded color index:%d, name %s coord:%.2f, %.2f, %.2f, %.2f: %.2f, %.2f, %.2f, %.2f\n",
                   i, name.C_Str(),
                   diffuseColor.x, diffuseColor.y, diffuseColor.z, diffuseColor.w,
                   ambientColor.x, ambientColor.y, ambientColor.z, ambientColor.w);
        }
    }

    return Ret;
}

void Mesh::Render()
{
    glEnableVertexAttribArray(m_positionLocation);
    glEnableVertexAttribArray(m_texCoordLocation);
    glEnableVertexAttribArray(m_normalLocation);
    
    for (unsigned int i = 0 ; i < m_Entries.size() ; i++) {
        glBindBuffer(GL_ARRAY_BUFFER, m_Entries[i].VB);
        glVertexAttribPointer(m_positionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
        glVertexAttribPointer(m_texCoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
        glVertexAttribPointer(m_normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Entries[i].IB);

        //starting from GL_TEXTURE1 to avoid conflict with GL_TEXTURE0 in the base texture.
        const unsigned int materialIndex = m_Entries[i].MaterialIndex;
        if( materialIndex < m_Materials.size() && m_Materials[materialIndex] ){
            m_Materials[materialIndex]->bind(GL_TEXTURE1, 1);
        }
        glDrawElements(GL_TRIANGLES, m_Entries[i].NumIndices, GL_UNSIGNED_INT, 0);
    }

    glDisableVertexAttribArray(m_positionLocation);
    glDisableVertexAttribArray(m_texCoordLocation);
    glDisableVertexAttribArray(m_normalLocation);
}
