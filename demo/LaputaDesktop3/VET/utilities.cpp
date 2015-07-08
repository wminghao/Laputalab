//
//  utilities.cpp
//  VET
//
//  Created by Xingze on 3/4/14.
//  Copyright (c) 2014 Xingze. All rights reserved.
//

#include "utilities.h"

void printPara(float P[])
{
    for (int i = 0; i < NP; i++){
        if (i < 3) cout << P[i]*180 << " ";
        else if (i == 3) cout << P[i]*V_WIDTH << " ";
        else if (i == 4) cout << P[i]*V_HEIGHT << " ";
        else cout << P[i]*DIST;
    }
    cout << endl;
    return;
}

float errorCalc(float sfImgDiff[][SF_WIDTH])
{
    float error = 0;
    
    for (int j = 0; j < SF_HEIGHT; j++)
        for (int i = 0; i < SF_WIDTH; i++){
            error += sfImgDiff[j][i]*sfImgDiff[j][i];
        }
    
    //cout << error << endl;
    
    return sqrt(error/(SF_HEIGHT*SF_WIDTH));
}

void drawSFImg(const char * winName, int xPos, int yPos, float sfImg[][SF_WIDTH])
{
    Mat img = Mat(SF_HEIGHT,SF_WIDTH,CV_32F, sfImg);
    Mat nImg;
    img.copyTo(nImg);
    normalize(nImg, nImg, 0, 1, CV_MINMAX);
    namedWindow(winName, WINDOW_AUTOSIZE);
    moveWindow(winName, xPos, yPos);
    imshow(winName, nImg);
    waitKey(1);
}

void createDepthMap(vector<myvec3> sfVertices, vector<triangle> faces, float dMap[][V_WIDTH])
{
    for (int i = 0; i < V_HEIGHT; i++)
        for (int j = 0; j < V_WIDTH; j++){
            dMap[i][j] = 10000;
        }
    
    for (int i = 0; i < faces.size(); i++){
        myvec3 v1 = sfVertices[faces[i].a];
        myvec3 v2 = sfVertices[faces[i].b];
        myvec3 v3 = sfVertices[faces[i].c];
        
        int maxX = max(max(v1.x, v2.x),v3.x);
        int maxY = max(max(v1.y, v2.y),v3.y);
        int minX = min(min(v1.x, v2.x),v3.x);
        int minY = min(min(v1.y, v2.y),v3.y);
        
        //cout << v1.z << " " << v2.z << " " << v3.z << endl;
        
        for (int jY = minY; jY <= maxY; jY++)
            for (int jX = minX; jX <= maxX; jX++){
                float denom = float((v2.y - v3.y)*(v1.x - v3.x) + (v3.x - v2.x)*(v1.y - v3.y));
                float a = float((v2.y - v3.y)*(jX - v3.x) + (v3.x - v2.x)*(jY - v3.y))/denom;
                float b = float((v3.y - v1.y)*(jX - v3.x) + (v1.x - v3.x)*(jY - v3.y))/denom;
                
                float c = 1 - a - b;
                
                if (0 <= a && a <= 1 && 0 <= b && b <= 1 && 0 <= c && c <= 1){
                    dMap[jY][jX] = a*v1.z + b*v2.z + c*v3.z;
                    //cout << dMap[jY][jX] << ", ";
                }
                
            }
        
    }
    
    /*
     // Display Depth Map
     Mat img = Mat(V_HEIGHT, V_WIDTH, CV_32F, dMap);
     //cout << img << endl;
     normalize(img, img, 0,1, CV_MINMAX);
     namedWindow("Depth Map", WINDOW_AUTOSIZE);
     imshow("Depth Map", img);
     waitKey(0);
     */
    
    return;
    
}

void genSFImg(Mat image, vector<triangle> & faces, vector<myvec3> & vertices, vector<myvec3> & sfVertices, unsigned char map[][SF_WIDTH], float sfImg[][SF_WIDTH])
{
    //IMPORTANT: consider sfImg normalization!
    for (int j = 0; j < SF_HEIGHT; j++)
        for (int i = 0; i < SF_WIDTH; i++){
            if (map[j][i] != 255){
                int faceIndex = map[j][i];
                myvec3 sfv1 = sfVertices[faces[faceIndex].a];
                myvec3 sfv2 = sfVertices[faces[faceIndex].b];
                myvec3 sfv3 = sfVertices[faces[faceIndex].c];
                
                myvec3 v1 = vertices[faces[faceIndex].a];
                myvec3 v2 = vertices[faces[faceIndex].b];
                myvec3 v3 = vertices[faces[faceIndex].c];
                
                float denom = float((sfv2.y - sfv3.y)*(sfv1.x - sfv3.x) + (sfv3.x - sfv2.x)*(sfv1.y - sfv3.y));
                float a = float((sfv2.y - sfv3.y)*(i - sfv3.x) + (sfv3.x - sfv2.x)*(j - sfv3.y))/denom;
                float b = float((sfv3.y - sfv1.y)*(i - sfv3.x) + (sfv1.x - sfv3.x)*(j - sfv3.y))/denom;
                
                float c = 1 - a - b;
                
                
                float v1x = (v1.x - V_WIDTH/2)/v1.z*FL + V_WIDTH/2;
                float v2x = (v2.x - V_WIDTH/2)/v2.z*FL + V_WIDTH/2;
                float v3x = (v3.x - V_WIDTH/2)/v3.z*FL + V_WIDTH/2;
                
                float v1y = (v1.y - V_HEIGHT/2)/v1.z*FL + V_HEIGHT/2;
                float v2y = (v2.y - V_HEIGHT/2)/v2.z*FL + V_HEIGHT/2;
                float v3y = (v3.y - V_HEIGHT/2)/v3.z*FL + V_HEIGHT/2;
                
                
                float tX = a*v1x + b*v2x + c*v3x;
                float tY = a*v1y + b*v2y + c*v3y;
                
                //interpolation
                float tX_f = floor(tX);
                float tX_c = ceil(tX);
                float tY_f = floor(tY);
                float tY_c = ceil(tY);
                float tmpx = tX - tX_f;
                float tmpy = tY - tY_f;
                
                sfImg[j][i] = image.at<unsigned char>(tY_f,tX_f)*(1-tmpx)*(1-tmpy) + image.at<unsigned char>(tY_f, tX_c)*(1-tmpy)*tmpx + image.at<unsigned char>(tY_c, tX_f)*(1-tmpx)*tmpy + image.at<unsigned char>(tY_c, tX_c)*tmpy*tmpx;
                
            }
            else
                continue;
        }
    
    //normalization(sfImg, map);
    
    return;
}

void genSFImg2(Mat image, vector<triangle> & faces, vector<myvec3> & vertices, vector<myvec3> & sfVertices, unsigned char map[][SF_WIDTH], float sfImg[][SF_WIDTH], Mat cam_int)
{
    //IMPORTANT: consider sfImg normalization!
    for (int j = 0; j < SF_HEIGHT; j++)
        for (int i = 0; i < SF_WIDTH; i++){
            if (map[j][i] != 255){
                int faceIndex = map[j][i];
                myvec3 sfv1 = sfVertices[faces[faceIndex].a];
                myvec3 sfv2 = sfVertices[faces[faceIndex].b];
                myvec3 sfv3 = sfVertices[faces[faceIndex].c];
                
                myvec3 v1 = vertices[faces[faceIndex].a];
                myvec3 v2 = vertices[faces[faceIndex].b];
                myvec3 v3 = vertices[faces[faceIndex].c];
                
                float denom = float((sfv2.y - sfv3.y)*(sfv1.x - sfv3.x) + (sfv3.x - sfv2.x)*(sfv1.y - sfv3.y));
                float a = float((sfv2.y - sfv3.y)*(i - sfv3.x) + (sfv3.x - sfv2.x)*(j - sfv3.y))/denom;
                float b = float((sfv3.y - sfv1.y)*(i - sfv3.x) + (sfv1.x - sfv3.x)*(j - sfv3.y))/denom;
                
                float c = 1 - a - b;
                
                
                float v1x = (650.66*v1.x + 319.50*v1.z)/v1.z;
                float v2x = (650.66*v2.x + 319.50*v2.z)/v1.z;
                float v3x = (650.66*v3.x + 319.50*v3.z)/v1.z;
                
                float v1y = (650.94*v1.y + 239.50*v1.z)/v1.z;
                float v2y = (650.94*v2.y + 239.50*v2.z)/v1.z;
                float v3y = (650.94*v3.y + 239.50*v3.z)/v1.z;
                
                float tX = a*v1x + b*v2x + c*v3x;
                float tY = a*v1y + b*v2y + c*v3y;
                
                //interpolation
                float tX_f = floor(tX);
                float tX_c = ceil(tX);
                float tY_f = floor(tY);
                float tY_c = ceil(tY);
                float tmpx = tX - tX_f;
                float tmpy = tY - tY_f;
                
                sfImg[j][i] = image.at<unsigned char>(tY_f,tX_f)*(1-tmpx)*(1-tmpy) + image.at<unsigned char>(tY_f, tX_c)*(1-tmpy)*tmpx + image.at<unsigned char>(tY_c, tX_f)*(1-tmpx)*tmpy + image.at<unsigned char>(tY_c, tX_c)*tmpy*tmpx;
                
            }
            else
                continue;
        }
    
    //normalization(sfImg, map);
    
    return;
}

void genSFImgGL(Mat image, vector<triangle> & faces, vector<myvec3> & vertices, vector<myvec3> & sfVertices, unsigned char map[][SF_WIDTH], float sfImg[][SF_WIDTH], Mat PMat)
{
    //IMPORTANT: consider sfImg normalization!
    for (int j = 0; j < SF_HEIGHT; j++)
        for (int i = 0; i < SF_WIDTH; i++){
            if (map[j][i] != 255){
                int faceIndex = map[j][i];
                myvec3 sfv1 = sfVertices[faces[faceIndex].a];
                myvec3 sfv2 = sfVertices[faces[faceIndex].b];
                myvec3 sfv3 = sfVertices[faces[faceIndex].c];
                
                myvec3 v1 = vertices[faces[faceIndex].a];
                myvec3 v2 = vertices[faces[faceIndex].b];
                myvec3 v3 = vertices[faces[faceIndex].c];
                
                float denom = float((sfv2.y - sfv3.y)*(sfv1.x - sfv3.x) + (sfv3.x - sfv2.x)*(sfv1.y - sfv3.y));
                float a = float((sfv2.y - sfv3.y)*(i - sfv3.x) + (sfv3.x - sfv2.x)*(j - sfv3.y))/denom;
                float b = float((sfv3.y - sfv1.y)*(i - sfv3.x) + (sfv1.x - sfv3.x)*(j - sfv3.y))/denom;
                
                float c = 1 - a - b;
                
                //Mat PMat = (Mat_<float>(3,4) << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0);
                
                Mat vertex3D_1 = (Mat_<float>(4,1) << v1.x, v1.y, v1.z, 1);
                Mat pt2D_1 = PMat*vertex3D_1;
                Mat vertex3D_2 = (Mat_<float>(4,1) << v2.x, v2.y, v2.z, 1);
                Mat pt2D_2 = PMat*vertex3D_2;
                Mat vertex3D_3 = (Mat_<float>(4,1) << v3.x, v3.y, v3.z, 1);
                Mat pt2D_3 = PMat*vertex3D_3;
                
                
                float v1x = pt2D_1.at<float>(0);
                float v2x = pt2D_2.at<float>(0);
                float v3x = pt2D_3.at<float>(0);
                
                float v1y = pt2D_1.at<float>(1);
                float v2y = pt2D_2.at<float>(1);
                float v3y = pt2D_3.at<float>(1);
                
                float tX = a*v1x + b*v2x + c*v3x;
                float tY = a*v1y + b*v2y + c*v3y;
                
                //interpolation
                float tX_f = floor(tX);
                float tX_c = ceil(tX);
                float tY_f = floor(tY);
                float tY_c = ceil(tY);
                float tmpx = tX - tX_f;
                float tmpy = tY - tY_f;
                
                sfImg[j][i] = image.at<unsigned char>(tY_f,tX_f)*(1-tmpx)*(1-tmpy) + image.at<unsigned char>(tY_f, tX_c)*(1-tmpy)*tmpx + image.at<unsigned char>(tY_c, tX_f)*(1-tmpx)*tmpy + image.at<unsigned char>(tY_c, tX_c)*tmpy*tmpx;
                
            }
            else
                continue;
        }
    
    //normalization(sfImg, map);
    
    return;
}



void drawMeshOnImg_Per(Mat image, vector<triangle> faces, vector<myvec3> vertices)
{
    for (int i = 0; i < faces.size(); i++){
        vector<Point> contour;
        myvec3 v_a = vertices[faces[i].a];
        myvec3 v_b = vertices[faces[i].b];
        myvec3 v_c = vertices[faces[i].c];
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

void drawMeshOnImg_Per2(Mat image, vector<triangle> faces, vector<myvec3> vertices, Mat cam_int) //With intrinsic parameters
{
    for (int i = 0; i < faces.size(); i++){
        vector<Point> contour;
        myvec3 v_a = vertices[faces[i].a];
        myvec3 v_b = vertices[faces[i].b];
        myvec3 v_c = vertices[faces[i].c];
        
        Mat vertex3D_1 = (Mat_<float>(3,1) << v_a.x, v_a.y, v_a.z);
        Mat pt2D_1 = cam_int*vertex3D_1;
        Mat vertex3D_2 = (Mat_<float>(3,1) << v_b.x, v_b.y, v_b.z);
        Mat pt2D_2 = cam_int*vertex3D_2;
        Mat vertex3D_3 = (Mat_<float>(3,1) << v_c.x, v_c.y, v_c.z);
        Mat pt2D_3 = cam_int*vertex3D_3;
        
        float v1x = pt2D_1.at<float>(0);
        float v2x = pt2D_2.at<float>(0);
        float v3x = pt2D_3.at<float>(0);
        
        float v1y = pt2D_1.at<float>(1);
        float v2y = pt2D_2.at<float>(1);
        float v3y = pt2D_3.at<float>(1);
        
        float v1z = pt2D_1.at<float>(2);
        float v2z = pt2D_2.at<float>(2);
        float v3z = pt2D_3.at<float>(2);
        
        contour.push_back(Point(v1x/v1z,v1y/v1z));
        contour.push_back(Point(v2x/v2z,v2y/v2z));
        contour.push_back(Point(v3x/v3z,v3y/v3z));
        
        
        const cv::Point *pts = (const cv::Point*) Mat(contour).data;
        int npts = Mat(contour).rows;
        
        polylines(image, &pts,&npts, 1,
                  true, 			// draw closed contour (i.e. joint end to start)
                  Scalar(0,0,255),// colour RGB ordering (here = green)
                  1, 		        // line thickness
                  CV_AA, 0);
        
    }
}

void drawGlassOnImg_Per(Mat image, vector<myvec3> verticesGlass)
{
    for (int i = 0; i < verticesGlass.size(); i = i+4){
        vector<Point> contour;
        myvec3 v_a = verticesGlass[i];
        myvec3 v_b = verticesGlass[i+1];
        myvec3 v_c = verticesGlass[i+2];
        myvec3 v_d = verticesGlass[i+3];
        
        
        contour.push_back(Point((v_a.x - V_WIDTH/2)/v_a.z*FL + V_WIDTH/2,(v_a.y - V_HEIGHT/2)/v_a.z*FL + V_HEIGHT/2));
        contour.push_back(Point((v_b.x - V_WIDTH/2)/v_b.z*FL + V_WIDTH/2,(v_b.y - V_HEIGHT/2)/v_b.z*FL + V_HEIGHT/2));
        contour.push_back(Point((v_c.x - V_WIDTH/2)/v_c.z*FL + V_WIDTH/2,(v_c.y - V_HEIGHT/2)/v_c.z*FL + V_HEIGHT/2));
        contour.push_back(Point((v_d.x - V_WIDTH/2)/v_d.z*FL + V_WIDTH/2,(v_d.y - V_HEIGHT/2)/v_d.z*FL + V_HEIGHT/2));
        
        
        const cv::Point *pts = (const cv::Point*) Mat(contour).data;
        int npts = Mat(contour).rows;
        
        polylines(image, &pts,&npts, 1,
                  true, 			// draw closed contour (i.e. joint end to start)
                  Scalar(20,20,20),// colour RGB ordering (here = green)
                  1, 		        // line thickness
                  CV_AA, 0);
        
    }
}

void drawGlassOnImg_Per_Real(Mat image, vector<myvec3> verticesGlass, float dMap[][V_WIDTH])
{
    for (int i = 0; i < verticesGlass.size()-12000; i = i+4){
        vector<Point> contour;
        myvec3 v_a = verticesGlass[i];
        myvec3 v_b = verticesGlass[i+1];
        myvec3 v_c = verticesGlass[i+2];
        myvec3 v_d = verticesGlass[i+3];
        
        float aX_p = (v_a.x - V_WIDTH/2)/v_a.z*FL + V_WIDTH/2;
        float aY_p = (v_a.y - V_HEIGHT/2)/v_a.z*FL + V_HEIGHT/2;
        float bX_p = (v_b.x - V_WIDTH/2)/v_b.z*FL + V_WIDTH/2;
        float bY_p = (v_b.y - V_HEIGHT/2)/v_b.z*FL + V_HEIGHT/2;
        float cX_p = (v_c.x - V_WIDTH/2)/v_c.z*FL + V_WIDTH/2;
        float cY_p = (v_c.y - V_HEIGHT/2)/v_c.z*FL + V_HEIGHT/2;
        float dX_p = (v_d.x - V_WIDTH/2)/v_d.z*FL + V_WIDTH/2;
        float dY_p = (v_d.y - V_HEIGHT/2)/v_d.z*FL + V_HEIGHT/2;
        
        
        if (dMap[int(aY_p)][int(aX_p)] < v_a.z && dMap[int(bY_p)][int(bX_p)] < v_b.z && dMap[int(cY_p)][int(cX_p)] < v_c.z && dMap[int(dY_p)][int(dX_p)] < v_d.z )
        {
            continue;
        }
        /*
         dMap[int(aY_p)][int(aX_p)] = 1100;
         dMap[int(bY_p)][int(bX_p)] = 1100;
         dMap[int(cY_p)][int(cX_p)] = 1100;
         dMap[int(dY_p)][int(dX_p)] = 1100;
         
         */
        
        contour.push_back(Point(aX_p,aY_p));
        contour.push_back(Point(bX_p,bY_p));
        contour.push_back(Point(cX_p,cY_p));
        contour.push_back(Point(dX_p,dY_p));
        
        
        const cv::Point *pts = (const cv::Point*) Mat(contour).data;
        int npts = Mat(contour).rows;
        
        polylines(image, &pts,&npts, 1,
                  true, 			// draw closed contour (i.e. joint end to start)
                  Scalar(20,20,20),// colour BGR ordering (here = green)
                  1, 		        // line thickness
                  CV_AA, 0);
        
    }
    
    /*
     // Display Depth Map
     Mat img = Mat(V_HEIGHT, V_WIDTH, CV_32F, dMap);
     //cout << img << endl;
     normalize(img, img, 0,1, CV_MINMAX);
     namedWindow("Depth Map", WINDOW_AUTOSIZE);
     imshow("Depth Map", img);
     waitKey(0);
     */
}

void drawGlassOnImg_Per_Real_RGB(Mat image, vector<myvec3> verticesGlass, float dMap[][V_WIDTH], int colR, int colG, int colB)
{
    for (int i = 0; i < verticesGlass.size()-12000; i = i+4){
        vector<Point> contour;
        myvec3 v_a = verticesGlass[i];
        myvec3 v_b = verticesGlass[i+1];
        myvec3 v_c = verticesGlass[i+2];
        myvec3 v_d = verticesGlass[i+3];
        
        float aX_p = (v_a.x - V_WIDTH/2)/v_a.z*FL + V_WIDTH/2;
        float aY_p = (v_a.y - V_HEIGHT/2)/v_a.z*FL + V_HEIGHT/2;
        float bX_p = (v_b.x - V_WIDTH/2)/v_b.z*FL + V_WIDTH/2;
        float bY_p = (v_b.y - V_HEIGHT/2)/v_b.z*FL + V_HEIGHT/2;
        float cX_p = (v_c.x - V_WIDTH/2)/v_c.z*FL + V_WIDTH/2;
        float cY_p = (v_c.y - V_HEIGHT/2)/v_c.z*FL + V_HEIGHT/2;
        float dX_p = (v_d.x - V_WIDTH/2)/v_d.z*FL + V_WIDTH/2;
        float dY_p = (v_d.y - V_HEIGHT/2)/v_d.z*FL + V_HEIGHT/2;
        
        
        if (dMap[int(aY_p)][int(aX_p)] < v_a.z && dMap[int(bY_p)][int(bX_p)] < v_b.z && dMap[int(cY_p)][int(cX_p)] < v_c.z && dMap[int(dY_p)][int(dX_p)] < v_d.z )
        {
            continue;
        }
        /*
         dMap[int(aY_p)][int(aX_p)] = 1100;
         dMap[int(bY_p)][int(bX_p)] = 1100;
         dMap[int(cY_p)][int(cX_p)] = 1100;
         dMap[int(dY_p)][int(dX_p)] = 1100;
         
         */
        
        contour.push_back(Point(aX_p,aY_p));
        contour.push_back(Point(bX_p,bY_p));
        contour.push_back(Point(cX_p,cY_p));
        contour.push_back(Point(dX_p,dY_p));
        
        
        const cv::Point *pts = (const cv::Point*) Mat(contour).data;
        int npts = Mat(contour).rows;
        
        polylines(image, &pts,&npts, 1,
                  true, 			// draw closed contour (i.e. joint end to start)
                  Scalar(colB,colG,colR),// colour BGR ordering (here = green)
                  1, 		        // line thickness
                  CV_AA, 0);
        
    }
    
    /*
     // Display Depth Map
     Mat img = Mat(V_HEIGHT, V_WIDTH, CV_32F, dMap);
     //cout << img << endl;
     normalize(img, img, 0,1, CV_MINMAX);
     namedWindow("Depth Map", WINDOW_AUTOSIZE);
     imshow("Depth Map", img);
     waitKey(0);
     */
}


void mytransform(vector<myvec3> & vertices, float P[])
{
    myrotateXYZ(vertices,P[0]*180,P[1]*180,P[2]*180);
    mytranslate(vertices, P[3]*V_WIDTH, P[4]*V_HEIGHT, P[5]*DIST);
    
}

void mytransformWithAU(vector<myvec3> & vertices, float P[], vector<myvec4> AU[]){
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < AU[i].size(); j++) {
            vertices[AU[i][j].v].x -= P[i+6]*AU[i][j].x;
            vertices[AU[i][j].v].y -= P[i+6]*AU[i][j].y;
            vertices[AU[i][j].v].z -= P[i+6]*AU[i][j].z;
        }
    }
    
    
    myrotateXYZ(vertices,P[0]*180,P[1]*180,P[2]*180);
    mytranslate(vertices, P[3]*V_WIDTH, P[4]*V_HEIGHT, P[5]*DIST);
}

void createV2FMap(vector<myvec3> sfVertices, vector<triangle> faces, unsigned char map[][SF_WIDTH])
{
    
    for (int i = 0; i < faces.size(); i++){
        myvec3 v1 = sfVertices[faces[i].a];
        myvec3 v2 = sfVertices[faces[i].b];
        myvec3 v3 = sfVertices[faces[i].c];
        
        int maxX = max(max(v1.x, v2.x),v3.x);
        int maxY = max(max(v1.y, v2.y),v3.y);
        int minX = min(min(v1.x, v2.x),v3.x);
        int minY = min(min(v1.y, v2.y),v3.y);
        
        for (int jY = minY; jY <= maxY; jY++)
            for (int jX = minX; jX <= maxX; jX++){
                float denom = float((v2.y - v3.y)*(v1.x - v3.x) + (v3.x - v2.x)*(v1.y - v3.y));
                float a = float((v2.y - v3.y)*(jX - v3.x) + (v3.x - v2.x)*(jY - v3.y))/denom;
                float b = float((v3.y - v1.y)*(jX - v3.x) + (v1.x - v3.x)*(jY - v3.y))/denom;
                
                float c = 1 - a - b;
                
                if (0 <= a && a <= 1 && 0 <= b && b <= 1 && 0 <= c && c <= 1){
                    map[jY][jX] = i;
                }
            }
        
    }
    
    /*
     // Display Map
     Mat img = Mat(SF_HEIGHT, SF_WIDTH, CV_8UC(1), map);
     //cout << img << endl;
     namedWindow("V2F Map", WINDOW_AUTOSIZE);
     imshow("V2F Map", img);
     waitKey(0);
     */
    
    return;
    
}

void loadAnimationUnit(const char * path, vector<myvec4> &AU, float xScale, float yScale, float zScale){
    
    ifstream ifs;
    ifs.open(path, ifstream::in);
    
    myvec4 elem;
    
    while (ifs >> elem.v >> elem.x >> elem.y >> elem.z){
        elem.x *= xScale;
        elem.y *= yScale;
        elem.z *= zScale;
        AU.push_back(elem);
    }
    
    ifs.close();
}

void adjustShape(float shapeUnits[], vector<myvec3> & vertices, float xScale, float yScale, float zScale) //To be optimized
{
    const string shapeUnitFile[SHAPEUNITS] = {
        pathPrefix + "demo/LaputaDesktop3/VET/facemodel/headheight_16.wfm",
        pathPrefix + "demo/LaputaDesktop3/VET/facemodel/eyebrowsvertical_8.wfm",
        pathPrefix + "demo/LaputaDesktop3/VET/facemodel/eyevertical_36.wfm",
        
        pathPrefix + "demo/LaputaDesktop3/VET/facemodel/eyeswidth_20.wfm",
        pathPrefix + "demo/LaputaDesktop3/VET/facemodel/eyesheight_24.wfm",
        pathPrefix + "demo/LaputaDesktop3/VET/facemodel/eyeseperation_36.wfm",
        
        pathPrefix + "demo/LaputaDesktop3/VET/facemodel/cheeksZ_2.wfm",
        pathPrefix + "demo/LaputaDesktop3/VET/facemodel/noseZExten_6.wfm",
        pathPrefix + "demo/LaputaDesktop3/VET/facemodel/nosevertical_17.wfm",
        
        pathPrefix + "demo/LaputaDesktop3/VET/facemodel/nosepointup_3.wfm",
        pathPrefix + "demo/LaputaDesktop3/VET/facemodel/mouthvertical_21.wfm",
        pathPrefix + "demo/LaputaDesktop3/VET/facemodel/mouthwidth_14.wfm",
        
        pathPrefix + "demo/LaputaDesktop3/VET/facemodel/eyesverticaldiff_36.wfm",
        pathPrefix + "demo/LaputaDesktop3/VET/facemodel/chinwidth_2.wfm",
        
    };
    
    
    for (int i = 0; i < SHAPEUNITS; i++){
        if (shapeUnits[i] != 0){
            ifstream ifs;
            ifs.open(shapeUnitFile[i],ifstream::in);
            
            myvec4 elem;
            
            while (ifs >> elem.v >> elem.x >> elem.y >> elem.z){
                vertices[elem.v].x -= shapeUnits[i]*elem.x;
                vertices[elem.v].y -= shapeUnits[i]*elem.y;
                vertices[elem.v].z -= shapeUnits[i]*elem.z;
            }
            
            ifs.close();
        }
        
    }
    
    myscale(vertices, xScale, yScale, zScale);
    
    return;
    
}


vector<myvec3> createSFVertices(vector<myvec3> & vertices)
{
    vector<myvec3> sfVertices(vertices);
    myscale(sfVertices, (SF_WIDTH-2)/(MAX_X-MIN_X), (SF_HEIGHT-2)/(MAX_Y-MIN_Y), 1);
    mytranslate(sfVertices, SF_WIDTH*abs(MIN_X)/(MAX_X-MIN_X), SF_HEIGHT*abs(MIN_Y)/(MAX_Y-MIN_Y), 0);
    
    return sfVertices;
}

void mytranslate(vector<myvec3> & vertices, float tx, float ty, float tz)
{
    for (int i = 0; i < vertices.size(); i++){
        vertices[i].x += tx;
        vertices[i].y += ty;
        vertices[i].z += tz;
    }
    
}

void myrotate(vector<myvec3> & vertices, float angle, int ux, int uy, int uz) // Rotate at origin point
{
    float cosa = cos(angle*PI/180);
    float sina = sin(angle*PI/180);
    
    for (int i = 0; i < vertices.size(); i++){
        float vx = vertices[i].x;
        float vy = vertices[i].y;
        float vz = vertices[i].z;
        vertices[i].x = vx*(cosa+ux*ux*(1-cosa))+vy*(ux*uy*(1-cosa)-uz*sina)+vz*(ux*uz*(1-cosa)+uy*sina);
        vertices[i].y = vx*(uy*ux*(1-cosa)+uz*sina)+vy*(cosa+uy*uy*(1-cosa))+vz*(uy*uz*(1-cosa)-ux*sina);
        vertices[i].z = vx*(uz*ux*(1-cosa)-uy*sina)+vy*(uz*uy*(1-cosa)+ux*sina)+vz*(cosa+uz*uz*(1-cosa));
    }
}

void myrotateXYZ(vector<myvec3> & vertices, float angleX, float angleY, float angleZ)
{
    float cosaX = cos(angleX*PI/180);
    float sinaX = sin(angleX*PI/180);
    float cosaY = cos(angleY*PI/180);
    float sinaY = sin(angleY*PI/180);
    float cosaZ = cos(angleZ*PI/180);
    float sinaZ = sin(angleZ*PI/180);
    
    for (int i = 0; i < vertices.size(); i++){
        float vx = vertices[i].x;
        float vy = vertices[i].y;
        float vz = vertices[i].z;
        float vx_new;
        float vy_new;
        float vz_new;
        
        //rotate about x-axis
        vy_new = cosaX*vy - sinaX*vz;
        vz_new = sinaX*vy + cosaX*vz;
        vy = vy_new;
        vz = vz_new;
        
        //rotate about y-axis
        vx_new = cosaY*vx + sinaY*vz;
        vz_new = -sinaY*vx + cosaY*vz;
        vx = vx_new;
        vz = vz_new;
        
        //rotate about z-axis
        vx_new = cosaZ*vx - sinaZ*vy;
        vy_new = sinaZ*vx + cosaZ*vy;
        vx = vx_new;
        vy = vy_new;
        
        vertices[i].x = vx;
        vertices[i].y = vy;
        vertices[i].z = vz;
        
    }
    
    
}

void myscale(vector<myvec3> & vertices, float sx, float sy, float sz)
{
    for (int i = 0; i < vertices.size(); i++){
        vertices[i].x *= sx;
        vertices[i].y *= sy;
        vertices[i].z *= sz;
    }
    
}

void drawVertices_Debug(vector<myvec3> &vertices, vector<triangle> &faces)
{
    Mat image(V_HEIGHT,V_WIDTH,CV_8UC3,Scalar(0,0,0));
    
    for (int i = 0; i < faces.size(); i++){
        vector<Point> contour;
        myvec3 v_a = vertices[faces[i].a];
        myvec3 v_b = vertices[faces[i].b];
        myvec3 v_c = vertices[faces[i].c];
        
        contour.push_back(Point(v_a.x,v_a.y));
        contour.push_back(Point(v_b.x,v_b.y));
        contour.push_back(Point(v_c.x,v_c.y));
        
        const cv::Point *pts = (const cv::Point*) Mat(contour).data;
        int npts = Mat(contour).rows;
        
        polylines(image, &pts,&npts, 1,
                  true, 			// draw closed contour (i.e. joint end to start)
                  Scalar(0,255,0),// colour RGB ordering (here = green)
                  1, 		        // line thickness
                  CV_AA, 0);
    }
    
    imshow("drawVertices", image);
    waitKey(0);
    
    return;
}

vector<triangle> readFaces(string faceFile)
{
    vector<triangle> faces;
    ifstream ifs;
    ifs.open(faceFile.c_str(), ifstream::in);
    
    triangle face;
    
    while (ifs >> face.a >> face.b >> face.c){
        faces.push_back(face);
    }
    
    ifs.close();
    
    return faces;
}

vector<myvec3> readVertices(string vertexFile)
{
    vector<myvec3> vertices;
    
    ifstream ifs;
    
    ifs.open(vertexFile.c_str(), ifstream::in);
    
    myvec3 vertex;
    
    while (ifs >> vertex.x >> vertex.y >> vertex.z){
        vertex.x = -vertex.x;
        vertex.y = -vertex.y;
        vertex.z = -vertex.z;
        vertices.push_back(vertex);
        //cout << vertex.x << " " << vertex.y << " " << vertex.z << endl;
    }
    
    ifs.close();
    
    return vertices;
}

Mat P2PMat(float P[]){
    Mat res = Mat::zeros(3, 4, CV_32F);
    float p0 = -P[0];
    res.at<float>(0,0) = cos(P[1]*PI)*cos(P[2]*PI);
    res.at<float>(0,1) = cos(p0*PI)*sin(P[2]*PI) + sin(p0*PI)*sin(P[1]*PI)*cos(P[2]*PI);
    res.at<float>(0,2) = sin(p0*PI)*sin(P[2]*PI) - cos(p0*PI)*sin(P[1]*PI)*cos(P[2]*PI);
    res.at<float>(0,3) = P[3]*V_WIDTH;
    res.at<float>(1,0) = -cos(P[1]*PI)*sin(P[2]*PI);
    res.at<float>(1,1) = cos(p0*PI)*cos(P[2]*PI) - sin(p0*PI)*sin(P[1]*PI)*sin(P[2]*PI);
    res.at<float>(1,2) = sin(p0*PI)*cos(P[2]*PI) + cos(p0*PI)*sin(P[1]*PI)*sin(P[2]*PI);
    res.at<float>(1,3) = -P[4]*V_HEIGHT;
    res.at<float>(2,0) = sin(P[1]*PI);
    res.at<float>(2,1) = -sin(p0*PI)*cos(P[1]*PI);
    res.at<float>(2,2) = cos(p0*PI)*cos(P[1]*PI);
    res.at<float>(2,3) = -P[5]*DIST;

    return res;
}

void PMat2P(Mat& PMat, float P[]){
    P[3] = PMat.at<float>(0,3);
    P[4] = PMat.at<float>(1,3);
    P[5] = PMat.at<float>(2,3);
    
    P[2] = atan(-PMat.at<float>(1,0)/PMat.at<float>(0,0));
    P[0] = atan(-PMat.at<float>(2,1)/PMat.at<float>(2,2));
    P[1] = atan(PMat.at<float>(2,0)/sqrt(PMat.at<float>(2,1)*PMat.at<float>(2,1) + PMat.at<float>(2,2)*PMat.at<float>(2,2)));
}

void mat4ToMat3x4(glm::mat4& pMat4, Mat* pMat3x4)
{
    //3 rows, 3 cols for x, y, z
    for (int i = 0; i < 4; ++i) {
        for(int j = 0; j< 3; ++j ) {
            pMat3x4->at<float>(j, i) = pMat4[i][j];
            //printf("mat4ToMat3x4: pMat4[%d][%d] = %f\r\n", i, j, pMat4[i][j]);
        }
    }
    /*
     for(int i = 0; i< 4; ++i ) {
     //printf("mat4ToMat3x4: pMat4[%d][%d] = %f\r\n", i, 3, pMat4[i][3]);
     }
     */
}
void mat3x4ToMat4(Mat* pMat3x4, glm::mat4& pMat4)
{
    //3 rows, 3 cols for x, y, z
    for (int i = 0; i < 3; ++i) {
        for(int j = 0; j< 4; ++j ) {
            pMat4[j][i] = pMat3x4->at<float>(i, j);
            //printf("mat3x4ToMat4: pMat4[%d][%d] = %f\r\n", i, j, pMat4[i][j]);
        }
    }
    pMat4[0][3] =  pMat4[1][3] = pMat4[2][3] = 0;
    pMat4[3][3] = 1;
}

