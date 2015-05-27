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

#include <opencv2/core/core.hpp>

#include "unit.h"

using namespace std;
using namespace cv;

class Candide3
{
public:
    Candide3(unsigned int width,
             unsigned int height,
             unsigned int fl): V_WIDTH(width), V_HEIGHT(height), FL(fl) {}
    bool readFaces(string& faceFile);
    bool readVertices(string& vertexFile);
    
    void drawMeshOnImg_Per(Mat& image);
private:
    vector<Triangle> faces;
    vector<Vector3f> vertices;
    
    unsigned int V_WIDTH;
    unsigned int V_HEIGHT;
    unsigned int FL;
};



#endif /* defined(__Laputa__Candide3__) */
