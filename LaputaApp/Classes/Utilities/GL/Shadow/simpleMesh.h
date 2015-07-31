//
//  simpleMesh.h
//  VET
//
//  Created by Howard Wang on 15-7-24.
//  Copyright (c) 2015年 Laputalab. All rights reserved.
//

#ifndef __VET__simpleMesh__
#define __VET__simpleMesh__

#include <stdio.h>
#include <map>
#include <vector>
#include "platform.h"

//math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//assimp library
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags

//include materials
#include "material.h"

#include "candide3.h"

class SimpleMesh
{
public:
    SimpleMesh();
    
    ~SimpleMesh();
    
    void setAttrUni(GLint texCountLocation,
                    GLint diffuseColorLocation,
                    GLint ambientColorLocation,
                    GLint specularColorLocation,
                    GLint textureImageLocation,
                    GLint envMapLocation,
                    GLint positionLocation,
                    GLint texCoordLocation,
                    GLint normalLocation) {
        //map to different uniforms and attributes
        m_texCountLocation = texCountLocation;
        m_diffuseColorLocation = diffuseColorLocation;
        m_ambientColorLocation = ambientColorLocation;
        m_specularColorLocation = specularColorLocation;
        m_textureImageLocation = textureImageLocation;
        m_envMapLocation = envMapLocation;
        m_positionLocation = positionLocation;
        m_texCoordLocation = texCoordLocation;
        m_normalLocation = normalLocation;
    }
    bool LoadMesh(const std::string& Filename, const char*candide3FacePath, const char* candide3VertPath, float zRotateInDegree,
                  bool bUploadCandide3Vertices, vector<myvec3>* candide3Vec);
    
    bool reloadMesh( const std::string& Filename, float zRotateInDegree ); //reload only the glasses, TODO, reset the ratio.
    
    void Render(GLuint textureObj);
    
    float getWidth() { return xMax-xMin;}
    
private:
    bool InitFromScene(const aiScene* pScene, const std::string& Filename, float zRotateInDegree);
    void InitMesh(unsigned int Index, const aiMesh* paiMesh, float zRotateInDegree);
    bool InitMaterials(const aiScene* pScene, const std::string& Filename);
    void Clear();
    
    int getMeshWidthInfo(const aiScene* pScene, const std::string& Filename);
    
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
    GLint m_specularColorLocation;
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
    
#if defined(DESKTOP_GL)
    // Vertex Array Objects Identifiers
    GLuint vao;
#endif //DESKTOP_GL
    
    float _candide3WidthRatio;  //width ratio from candide3 to glasses
};

#endif /* defined(__VET__simpleMesh__) */
