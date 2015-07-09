//
//  main.h
//  VET
//
//  Created by Xingze on 3/4/14.
//  Copyright (c) 2014 Xingze. All rights reserved.
//

#ifndef VET_main_h
#define VET_main_h

#include <string>

using namespace std;


#define PI 3.14159265
#define NP 6 //number of parameters to track
#define MAX_X 0.61
#define MIN_X -0.61
#define MAX_Y 0.852
#define MIN_Y -1.061
#define V_WIDTH 640
#define V_HEIGHT 480
#define SHAPEUNITS 14
#define DIST 600 //to be verified, need to find the correct value
#define FL 500 //to be verified, need to find the correct value

#define SF_WIDTH 40
#define SF_HEIGHT 40
#define MAX_ITER 40
//#define K 2
#define MIN_ITER 20

typedef struct{
    unsigned int a;
    unsigned int b;
    unsigned int c;
} triangle;

extern string path;


#endif