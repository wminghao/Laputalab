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

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#ifdef CV3
#include <opencv2/xfeatures2d.hpp>
#else
#include <opencv2/nonfree/nonfree.hpp>
#endif

bool Candide3::readFaces(string& faceFile)
{
    ifstream ifs;
    ifs.open(faceFile.c_str(), ifstream::in);
    
    Triangle face;
    
    while (ifs >> face.a >> face.b >> face.c){
        faces.push_back(face);
    }
    
    ifs.close();
    
    return true;
}

bool Candide3::readVertices(string& vertexFile)
{
    ifstream ifs;
    
    ifs.open(vertexFile.c_str(), ifstream::in);
    
    Vector3f vertex;
    
    while (ifs >> vertex.x >> vertex.y >> vertex.z){
        vertex.x = -vertex.x;
        vertex.y = -vertex.y;
        vertex.z = -vertex.z;
        vertices.push_back(vertex);
        //cout << vertex.x << " " << vertex.y << " " << vertex.z << endl;
    }
    
    ifs.close();
    
    return true;
}

void Candide3::drawMeshOnImg_Per(Mat& image)
{
    size_t noOfFaces = faces.size();
    for (int i = 0; i < noOfFaces; i++){
        vector<Point> contour;
        Vector3f v_a = vertices[faces[i].a];
        Vector3f v_b = vertices[faces[i].b];
        Vector3f v_c = vertices[faces[i].c];
        /*
         contour.push_back(Point((v_a.x - 320)/v_a.z*DIST + 320,(v_a.y - 240)/v_a.z*DIST + 240));
         contour.push_back(Point((v_b.x - 320)/v_b.z*DIST + 320,(v_b.y - 240)/v_b.z*DIST + 240));
         contour.push_back(Point((v_c.x - 320)/v_c.z*DIST + 320,(v_c.y - 240)/v_c.z*DIST + 240));
         */
        contour.push_back(Point((v_a.x - V_WIDTH/2)/v_a.z*FL + V_WIDTH/2,(v_a.y - V_HEIGHT/2)/v_a.z*FL + V_HEIGHT/2));
        contour.push_back(Point((v_b.x - V_WIDTH/2)/v_b.z*FL + V_WIDTH/2,(v_b.y - V_HEIGHT/2)/v_b.z*FL + V_HEIGHT/2));
        contour.push_back(Point((v_c.x - V_WIDTH/2)/v_c.z*FL + V_WIDTH/2,(v_c.y - V_HEIGHT/2)/v_c.z*FL + V_HEIGHT/2));
        
        
        const cv::Point *pts = (const cv::Point*) Mat(contour).data;
        int npts = Mat(contour).rows;
        
        polylines(image, &pts,&npts, 1,
                  true, 			// draw closed contour (i.e. joint end to start)
                  Scalar(0,255,0),// colour RGB ordering (here = green)
                  1, 		        // line thickness
                  CV_AA, 0);
        
    }
}
