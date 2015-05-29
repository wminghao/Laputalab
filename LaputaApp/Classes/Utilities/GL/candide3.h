//
//  Candide3.h
//  Laputa
//
//  Created by Howard Wang on 15-5-27.
//
//

#ifndef __Laputa__Candide3__
#define __Laputa__Candide3__

#include <stdio.h>
#include <vector>
#include <string>

#include "platform.h"
#include "unit.h"
#include "candide3Texture.h"

using namespace std;

/*
 There is only 1 mesh, with multiple faces(triangles), and multiple vertices
 */

class Candide3
{
public:
    Candide3(unsigned int width,
             unsigned int height,
             unsigned int fl);

    void setAttrUni(GLint texCountLocation,
                    GLint textureImageLocation,
                    GLint positionLocation,
                    GLint texCoordLocation,
                    GLint normalLocation);
    
    bool readFaces(string& faceFile);
    bool readVertices(string& vertexFile);
    
    void render(GLuint textureObj);
private:
    vector<unsigned int> indices;
    vector<Vertex> vertices;
    
    unsigned int V_WIDTH;
    unsigned int V_HEIGHT;
    unsigned int FL;
    
    //shader location
    GLint m_positionLocation;
    GLint m_texCoordLocation;
    GLint m_normalLocation;
    
    //texture
    Candide3Texture* candide3Texture;
    
    //vertices and faces
    GLuint VB;
    GLuint IB;
    unsigned int NumIndices;    
};



#endif /* defined(__Laputa__Candide3__) */
