//
//  LandMark.h
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

using namespace std;
using namespace cv;

class LandMark
{
 public:
    LandMark(); //load the model
    ~LandMark(){}
    int ProcessImage(string& faceImg, string& jsonStr);
    
 private:
    Mat models;
    Mat lms0;
    vector<KeyPoint> kps0;
    CascadeClassifier face;
    SiftDescriptorExtractor extractor;
    Mat descriptor;    
};
