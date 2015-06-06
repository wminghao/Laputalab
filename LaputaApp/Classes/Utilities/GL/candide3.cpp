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

const float DELTA_BEHIND_GLASSES = 4.0; //delta face behind the glasses

#ifdef DESKTOP_MAC
const float DELTA_SMALLER_GLASSES = 1.0; //delta face width smaller than glasses
#else
const float DELTA_SMALLER_GLASSES = -5.0; //delta face width smaller than glasses
#endif

//Xingze's head
//Shape Factors
const int SHAPEUNITS = 14;
const float shapeFactor[SHAPEUNITS] = //for Xz
    {0.0f,0.5f,0.4f, //headheight, eyebrows vertical pos, eyes vertical pos;
    0.1f,0.0f,0.3f, // eyes width, eyes height, eye seperation dis;
    0.0f,1.0f,0.5f, // cheeks z, nose-z extension, nose vertical pos;
    0.0f,0.0f,0.3f, // nose pointing up, mouth vertical pos, mouth width;
    0.0f,0.0f}; // eyes vertical diff, chin width;

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

bool Candide3::readVertices(string& vertexFile, float glassesWidth, float zRotateInDegree)
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
        if( zRotateInDegree == 90 ) {
            //aspect ratio is 16/9 for 90 mode
            texture.x = (1-(vert.y+1)/2)*9/16;
            texture.y = (vert.x+1)/2;
        } else {
            texture.x = (vert.x+1)/2;
            texture.y = (vert.y+1)/2;
        }
        
        vert.x *= ratio;
        vert.y *= ratio;
        //vert.z = -DELTA_BEHIND_GLASSES;
        vert.z = vert.z*ratio - DELTA_BEHIND_GLASSES;
        
        Vertex v(vert, texture, normal);
        
        vertices.push_back(v);
        cout << "vert: "<<vert.x << " " << vert.y << " " << vert.z << " texture: "<<texture.x<<" "<<texture.y<<endl;
    }
    
    ifs.close();
    
    //adjust the sape
    //adjustShape(NULL, 0, shapeFactor, 1.0, 1.0, 1.0);
    
    cout << "Total vertices: " << vertices.size() <<endl;
    
    glGenBuffers(1, &VB);
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
    
    return true;
}

//set the candide3 vertices from
float Candide3::setCandide3Vertices(vector<myvec3>* vec, float zRotateInDegree)
{
    float xMin = 0;
    float xMax = 0;
    float yMin = 0;
    float yMax = 0;
    float zMin = 0;
    float zMax = 0;
    size_t total = vec->size();
    vertices.resize(total);
    for (int i = 0; i < total ; i++){
        myvec3 vert = (*vec)[i];
        vertices[i].m_pos.x = vert.x;
        vertices[i].m_pos.y = -vert.y; //it's up side down
        vertices[i].m_pos.z = vert.z;
        
        if( vert.x > xMax ) {
            xMax = vert.x;
        }
        if( vert.x < xMin ) {
            xMin = vert.x;
        }
        if( vert.y > yMax ) {
            yMax = vert.y;
        }
        if( vert.y < yMin ) {
            yMin = vert.y;
        }
        if( vert.z > zMax ) {
            zMax = vert.z;
        }
        if( vert.z < zMin ) {
            zMin = vert.z;
        }
        cout << "candide3 vert "<<i<<" : "<<vertices[i].m_pos.x << " " << vertices[i].m_pos.y << " " << vertices[i].m_pos.z<<endl;
    }
    float width = (xMax - xMin);
    cout<<"New xMax="<<xMax<<" xMin="<<xMin << " candide3 width = " << (xMax-xMin)<<endl;
    cout<<"New yMax="<<yMax<<" yMin="<<yMin << " candide3 height = " << (yMax-yMin)<<endl;
    cout<<"New zMax="<<zMax<<" zMin="<<zMin << " candide3 depth = " << (zMax-zMin)<<endl;
    
    
    for (int i = 0; i < total ; i++){
        //map directly into texture
        if( zRotateInDegree == 90 ) {
            //aspect ratio is 16/9 for 90 mode
            vertices[i].m_tex.x = 1-(vertices[i].m_pos.y/width+1)/2;
            vertices[i].m_tex.y = (vertices[i].m_pos.x/width+1)/2;
        } else {
            vertices[i].m_tex.x = (vertices[i].m_pos.x/width+1)/2;
            vertices[i].m_tex.y = (vertices[i].m_pos.y/width+1)/2;
        }
        cout << "vert_tex "<<i<<" : "<<vertices[i].m_tex.x << " " << vertices[i].m_tex.y<<endl;
    }
    
    glGenBuffers(1, &VB);
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
    return width;
}
/*
void Candide3::adjustShape(const char**shapeUnitFile, int totalShapeUnits, const float shapeUnits[], float xScale, float yScale, float zScale) //To be optimized
{
    const char * shapeUnitFiles[SHAPEUNITS] = {
        "/Users/howard/AR_lib/LaputaDesktop2/VET/facemodel/headheight_16.wfm",
        "/Users/howard/AR_lib/LaputaDesktop2/VET/facemodel/eyebrowsvertical_8.wfm",
        "/Users/howard/AR_lib/LaputaDesktop2/VET/facemodel/eyevertical_36.wfm",
        
        "/Users/howard/AR_lib/LaputaDesktop2/VET/facemodel/eyeswidth_20.wfm",
        "/Users/howard/AR_lib/LaputaDesktop2/VET/facemodel/eyesheight_24.wfm",
        "/Users/howard/AR_lib/LaputaDesktop2/VET/facemodel/eyeseperation_36.wfm",
        
        "/Users/howard/AR_lib/LaputaDesktop2/VET/facemodel/cheeksZ_2.wfm",
        "/Users/howard/AR_lib/LaputaDesktop2/VET/facemodel/noseZExten_6.wfm",
        "/Users/howard/AR_lib/LaputaDesktop2/VET/facemodel/nosevertical_17.wfm",
        
        "/Users/howard/AR_lib/LaputaDesktop2/VET/facemodel/nosepointup_3.wfm",
        "/Users/howard/AR_lib/LaputaDesktop2/VET/facemodel/mouthvertical_21.wfm",
        "/Users/howard/AR_lib/LaputaDesktop2/VET/facemodel/mouthwidth_14.wfm",
        
        "/Users/howard/AR_lib/LaputaDesktop2/VET/facemodel/eyesverticaldiff_36.wfm",
        "/Users/howard/AR_lib/LaputaDesktop2/VET/facemodel/chinwidth_2.wfm",
        
    };
    for (int i = 0; i < SHAPEUNITS; i++){
        if (shapeUnits[i] != 0){
            ifstream ifs;
            ifs.open(shapeUnitFiles[i],ifstream::in);
            
            myvec4 elem;
            
            while (ifs >> elem.v >> elem.x >> elem.y >> elem.z){
                vertices[elem.v].m_pos.x -= shapeUnits[i]*elem.x;
                vertices[elem.v].m_pos.y -= shapeUnits[i]*elem.y;
                vertices[elem.v].m_pos.z -= shapeUnits[i]*elem.z;
            }
            
            ifs.close();
        }
        
    }
    
    for (int i = 0; i < vertices.size(); i++){
        vertices[i].m_pos.x *= xScale;
        vertices[i].m_pos.y *= yScale;
        vertices[i].m_pos.z *= zScale;
        //cout << "vert "<<i<<" : "<<vertices[i].m_pos.x << " " << vertices[i].m_pos.y << " " << vertices[i].m_pos.z<<endl;
    }
    
    return;
}
*/

void Candide3::render(GLuint textureObj)
{
    glDisable( GL_CULL_FACE );
    
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glVertexAttribPointer(m_positionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); //3*4
    glVertexAttribPointer(m_texCoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12); //2*4
    glVertexAttribPointer(m_normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20); //3*4
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    
    //starting from GL_TEXTURE1 to avoid conflict with GL_TEXTURE0 in the base texture.
    candide3Texture->bind(1, textureObj);
    
    //Use GL_TRIANGLE_FAN instead of GL_TRIANGLES
    glDrawElements(GL_TRIANGLES, NumIndices, GL_UNSIGNED_INT, 0);
    
    glEnable( GL_CULL_FACE );
}
