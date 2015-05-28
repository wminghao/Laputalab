//
//  Candide3.cpp
//  Laputa
//
//  Created by Howard Wang on 15-5-27.
//
//

#include "Candide3.h"

#include<iostream>
#include<fstream>

Candide3::Candide3(unsigned int width,
         unsigned int height,
         unsigned int fl): V_WIDTH(width), V_HEIGHT(height), FL(fl) {
#ifdef DESKTOP_MAC
    glGenVertexArrays(1, &vao);
#endif
}

void Candide3::setAttrUni(GLint texCountLocation,
                          GLint textureImageLocation,
                          GLint positionLocation,
                          GLint texCoordLocation,
                          GLint normalLocation) {
    //map to different uniforms and attributes
    m_positionLocation = positionLocation;
    m_texCoordLocation = texCoordLocation;
    m_normalLocation = normalLocation;
    
    candide3Texture = new Candide3Texture(texCountLocation, textureImageLocation);
}

bool Candide3::readFaces(string& faceFile)
{
    ifstream ifs;
    ifs.open(faceFile.c_str(), ifstream::in);
    
    Triangle face; //3 points per face
    
    while (ifs >> face.a >> face.b >> face.c){
        indices.push_back(face.a);
        indices.push_back(face.b);
        indices.push_back(face.c);
    }
    
    ifs.close();
    
    //load into 
    NumIndices = (unsigned int)indices.size();
    
    glGenBuffers(1, &IB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * NumIndices, &indices[0], GL_STATIC_DRAW);

    return true;
}

bool Candide3::readVertices(string& vertexFile)
{
    ifstream ifs;
    
    ifs.open(vertexFile.c_str(), ifstream::in);
    
    Vector3f vert;
    Vector2f texture;
    Vector3f normal;
    while (ifs >> vert.x >> vert.y >> vert.z){
        vert.x = -vert.x;
        vert.y = -vert.y;
        vert.z = -vert.z;
        
        //TODO
        texture.x = 0;
        texture.y = 0;
        
        Vertex v(vert, texture, normal);
        
        vertices.push_back(v);
        //cout << vert.x << " " << vert.y << " " << vert.z << endl;
    }
    
    ifs.close();

    glGenBuffers(1, &VB);
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
    
    return true;
}

void Candide3::draw(GLuint textureObj)
{
    
#ifdef DESKTOP_MAC
    //according to http://stackoverflow.com/questions/24643027/opengl-invalid-operation-following-glenablevertexattribarray
    //enable core profile
    glBindVertexArray( vao );
#endif
    
    glEnableVertexAttribArray(m_positionLocation);
    glEnableVertexAttribArray(m_texCoordLocation);
    glEnableVertexAttribArray(m_normalLocation);
    
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glVertexAttribPointer(m_positionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); //3*4
    glVertexAttribPointer(m_texCoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12); //2*4
    glVertexAttribPointer(m_normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20); //3*4
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    
    //starting from GL_TEXTURE2 to avoid conflict with GL_TEXTURE0 & GL_TEXTURE1 in the base texture.
    candide3Texture->bind(GL_TEXTURE2, 1, textureObj);
    
    glDrawElements(GL_TRIANGLES, NumIndices, GL_UNSIGNED_INT, 0);
    
    glDisableVertexAttribArray(m_positionLocation);
    glDisableVertexAttribArray(m_texCoordLocation);
    glDisableVertexAttribArray(m_normalLocation);
}
