//
//  Candide3.cpp
//  Laputa
//
//  Created by Howard Wang on 15-5-27.
//
//

#include "Candide3.h"
#include "err.h"

#include<iostream>
#include<fstream>

const float DELTA_BEHIND_GLASSES = 5.0; //delta face behind the glasses
const float DELTA_SMALLER_GLASSES = 1.0; //delta face width smaller than glasses

void Candide3::setAttrUni(GLint texCountLocation,
                          GLint textureImageLocation,
                          GLint positionLocation,
                          GLint texCoordLocation,
                          GLint normalLocation) {
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
        cout << "faces: "<<face.a << " " << face.b << " " << face.c <<endl;
    }
    
    ifs.close();
    
    //load into 
    NumIndices = (unsigned int)indices.size();
    
    cout << "Total faces: " << NumIndices/3 <<endl;
    
    glGenBuffers(1, &IB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * NumIndices, &indices[0], GL_STATIC_DRAW);

    return true;
}

bool Candide3::readVertices(string& vertexFile, float glassesWidth)
{
    ifstream ifs;
    
    ifs.open(vertexFile.c_str(), ifstream::in);
    
    Vector3f vert;
    Vector2f texture;
    Vector3f normal;
    
    float xMin = 0;
    float xMax = 0;
    
    //first read the width of the mesh
    while (ifs >> vert.x >> vert.y >> vert.z){
        if( vert.x > xMax ) {
            xMax = vert.x;
        }
        if( vert.x < xMin ) {
            xMin = vert.x;
        }
    }
    
    float width = xMax - xMin;
    float ratio = (glassesWidth-DELTA_SMALLER_GLASSES)/width; //inside the width
    
    cout <<"candide width=" << width << " glasses width="<<glassesWidth << " ratio="<<ratio<<endl;
    
    //then seek to the beginning
    ifs.clear();
    ifs.seekg(0, ios::beg);
    while (ifs >> vert.x >> vert.y >> vert.z){
        //map directly into texture
        texture.x = (vert.x+1)/2;
        texture.y = (vert.y+1)/2;
        
        vert.x *= ratio;
        vert.y *= ratio;
        //vert.z = -DELTA_BEHIND_GLASSES;
        vert.z = vert.z*ratio - DELTA_BEHIND_GLASSES;
        
        Vertex v(vert, texture, normal);
        
        vertices.push_back(v);
        cout << "vert: "<<vert.x << " " << vert.y << " " << vert.z << " texture: "<<texture.x<<" "<<texture.y<<endl;
    }
    
    ifs.close();
    
    cout << "Total vertices: " << vertices.size() <<endl;
    
    glGenBuffers(1, &VB);
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
    
    return true;
}

void Candide3::render(GLuint textureObj)
{    
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glVertexAttribPointer(m_positionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); //3*4
    glVertexAttribPointer(m_texCoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12); //2*4
    glVertexAttribPointer(m_normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20); //3*4
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    
    //starting from GL_TEXTURE1 to avoid conflict with GL_TEXTURE0 in the base texture.
    candide3Texture->bind(GL_TEXTURE1, 1, textureObj);
    
    //Use GL_TRIANGLE_FAN instead of GL_TRIANGLES
    glDrawElements(GL_TRIANGLE_FAN, NumIndices, GL_UNSIGNED_INT, 0);
}
