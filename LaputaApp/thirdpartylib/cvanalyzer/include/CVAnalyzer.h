//
//  CVAnalyzerLib.h
//  Laputa
//
//  Created by Howard Wang on 15-5-5.
//
//

#ifndef __Laputa__CVAnalyzer__
#define __Laputa__CVAnalyzer__

//#define CV3

#include <CoreVideo/CVPixelBuffer.h>
#include <iostream>
#include <fstream>
#include <string>
#include "dirent.h"

#include <vector>
#include <queue>
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#ifdef CV3
#include <opencv2/xfeatures2d.hpp>
#else
#include <opencv2/nonfree/nonfree.hpp>
#endif

#define PCADIM 20
#define FEATUREDIM 128


using namespace std;
using namespace cv;

class CVAnalyzer
{
    
public:
    CVAnalyzer();
    bool init(const char* prefix);
    bool processImage(Mat& frame, bool onTap);
    
private:
    //constant
    const int numLMs = 68;
    const int numModels = 4;
    const int rowsM = FEATUREDIM * numLMs;
    const int colsM = numLMs * 2;
    
    //Load model
    Mat models;
    Mat coeff_pca;
    Mat lmsMean;
    vector<KeyPoint> kpsMean;
    Mat lms0;
    vector<KeyPoint> kps0;
    CascadeClassifier face;
    
#ifdef CV3
    cv::Ptr<Feature2D> extractor = xfeatures2d::SIFT::create();
#else
    SiftDescriptorExtractor extractor;    
#endif

    Mat descriptor;
    
    int trackFlag = 0;
    int numFrame = 0;
    Mat lms;
    vector<KeyPoint> kps;
    Mat meanDescriptor;
    Mat meanDescriptorRef;
    queue<Mat> desQueue;
    queue<Mat> desQueueRef;
};

#endif /* defined(__Laputa__CVAnalyzer__) */
