//
//  OglImageAPI.h
//  FaceLM
//
//  Created by Xingze & Minghao on 4/22/15.
//  Copyright (c) 2015 laputalab. All rights reserved.
//
#include <iostream>
#include <fstream>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/objdetect/objdetect.hpp>

//math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace cv;
using namespace glm;

class OglImageAPI
{
 public:
    OglImageAPI(); //load the model
    ~OglImageAPI(){}
    int ProcessImage(string& faceImg, 
                     mat4& model,
                     string& errReason);
 private:
    Mat models;
    Mat lms0;
    vector<KeyPoint> kps0;
    CascadeClassifier face;
    SiftDescriptorExtractor extractor;
    Mat descriptor;    
};
