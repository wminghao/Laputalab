//
//  utilities.h
//  VET
//
//  Created by Xingze on 3/4/14.
//  Copyright (c) 2014 Xingze. All rights reserved.
//

#ifndef __VET__utilities__
#define __VET__utilities__

#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <math.h>
#include "main.h"
#include "unit.h"

//math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef __MACH__
const string pathPrefix = "/Users/howard/AR/";
#else
const string pathPrefix = "/Laputalab/";
#endif //__MACH__
using namespace std;
using namespace cv;

vector<myvec3> readVertices(string inFile);
vector<triangle> readFaces(string faceFile);
void drawVertices_Debug(vector<myvec3> &vertices, vector<triangle> &faces);
void myscale(vector<myvec3> & vertices, float sx, float sy, float sz);
void myrotate(vector<myvec3> & vertices, float angle, int ux, int uy, int uz);
void myrotateXYZ(vector<myvec3> & vertices, float angleX, float angleY, float angleZ);
void mytranslate(vector<myvec3> & vertices, float tx, float ty, float tz);
vector<myvec3> createSFVertices(vector<myvec3> & vertices);
void loadAnimationUnit(const char * path, vector<myvec4> &AU, float xScale, float yScale, float zScale);
void adjustShape(float shapeUnits[], vector<myvec3> & vertices, float xScale, float yScale, float zScale);
void createV2FMap(vector<myvec3> sfVertices, vector<triangle> faces, unsigned char map[][SF_WIDTH]);
void mytransform(vector<myvec3> & vertices, float P[]);
void mytransformWithAU(vector<myvec3> & vertices, float P[], vector<myvec4> AU[]);
void drawGlassOnImg_Per(Mat image, vector<myvec3> verticesGlass);
void drawMeshOnImg_Per(Mat image, vector<triangle> faces, vector<myvec3> vertices);
void drawMeshOnImg_Per2(Mat image, vector<triangle> faces, vector<myvec3> vertices, Mat cam_int);
void drawGlassOnImg_Per_Real_RGB(Mat image, vector<myvec3> verticesGlass, float dMap[][V_WIDTH], int colR, int colG, int colB);
void drawGlassOnImg_Per_Real(Mat image, vector<myvec3> verticesGlass, float dMap[][V_WIDTH]);
void genSFImg(Mat image, vector<triangle> & faces, vector<myvec3> & vertices, vector<myvec3> & sfVertices, unsigned char map[][SF_WIDTH], float sfImg[][SF_WIDTH]);
void genSFImg2(Mat image, vector<triangle> & faces, vector<myvec3> & vertices, vector<myvec3> & sfVertices, unsigned char map[][SF_WIDTH], float sfImg[][SF_WIDTH], Mat cam_int);
void genSFImgGL(Mat image, vector<triangle> & faces, vector<myvec3> & vertices, vector<myvec3> & sfVertices, unsigned char map[][SF_WIDTH], float sfImg[][SF_WIDTH], Mat PMat);
void createDepthMap(vector<myvec3> sfVertices, vector<triangle> faces, float dMap[][V_WIDTH]);
void drawSFImg(const char * winName, int xPos, int yPos, float sfImg[][SF_WIDTH]);

float errorCalc(float sfImgDiff[][SF_WIDTH]);
void printPara(float P[]);
Mat P2PMat(float P[]);
void PMat2P(Mat PMat, float P[]);

void mat4ToMat3x4(glm::mat4& pMat4, Mat* pMat3x4);
void mat3x4ToMat4(Mat* pMat3x4, glm::mat4& pMat4);

#endif /* defined(__VET__utilities__) */
