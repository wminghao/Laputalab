//
//  OglImageAPI.cpp
//  FaceLM
//
//  Created by Xingze and Minghao on 4/21/15.
//  Copyright (c) 2015 laputalab. All rights reserved.
//
#include "OglImageAPI.h"
#include <sys/ioctl.h>
#include <net/if.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

#define KPSIZE 6
#define DEPTH 600

using namespace glm;

//in the future, add more machines to the list, the last item in the list is always 0 to indicate end of the array
const char* SELECTED_MAC_ADDRESS[]= {"0242ac110001", /*ytiyan web server docker instance*/
                                     "02d9e60ee2fe", /*test machine*/
                                     0};

const int NO_ERROR = 0;
const int UNRECOGNIZED_IMAGE_FORMAT = -1;
const int UNRECOGNIZED_FACE_NUMBER = -2;
const int UNRECOGNIZED_MAC_ADDR = -1000;

const static int numLMs = 68;
const static int numModels = 4;
const static int rowsM = 128 * numLMs;
const static int colsM = numLMs * 2;
const static int numIter = 1;

void loadModel(const char * modelFile, int rows, int cols, Mat data);
vector<KeyPoint> loadLandmarks(const char * lmFile, int numLM);
void kps2mat(vector<KeyPoint> &kps, Mat lms, int numLMs);
void mat2kps(Mat lms, vector<KeyPoint> &kps, int numLMs);
void transTemplate(Mat lms, int numLMs, float shiftX, float shiftY);
void drawPoints(Mat image, vector<KeyPoint> kps, Scalar color);

//namespace to avoid collision
namespace OglImageAPI_NameSpace{

#define NUMLMS 68
#define RM 0.4
#define RN 0.6
#define RG 1.2
#define PI 3.141592653
#define SHAPEUNITS 14
#define NP 6
#define DIST 500
#define INITYSHIFT -0.35
#define INITZSHIFT -0.13 

typedef struct{
    float x;
    float y;
    float z;
} myvec3;

typedef struct{
    unsigned int a;
    unsigned int b;
    unsigned int c;
} triangle;

typedef struct{
    int v;
    float x;
    float y;
    float z;
} myvec4;

vector<Point2f> mat2Point2f(Mat lms);
vector<triangle> readFaces(string faceFile);
vector<myvec3> readVertices(string vertexFile);
void mytranslate(vector<myvec3> & vertices, float tx, float ty, float tz);
void myrotate(vector<myvec3> & vertices, float angle, float ux, float uy, float uz);
void myrotateXYZ(vector<myvec3> & vertices, float angleX, float angleY, float angleZ);
void myrotateWithMat(vector<myvec3> & vertices, Mat rotMat);
void mytransform(vector<myvec3> & vertices, float P[]);
void myscale(vector<myvec3> & vertices, float sx, float sy, float sz);
void poseEstimate_3D(Mat img, float rmRatio, vector<Point2f> &landmarks, Mat &rotAxis, float &rotAngle, Mat &newNormal, float &rotAngle_newNormal);
float angleCalc(Point2f pt1, Point2f pt2);
float estimateRM(vector<Point2f> &landmarks);
mat4 genRT(Mat& img, vector<myvec3> &verticesM, vector<triangle> &faces, float rmRatio, vector<Point2f> &landmarks, Mat &rotAxis, float &rotAngle, Mat &newNormal, float &rotAngle_newNormal);
void PMat2P(Mat& PMat, float P[]);
}

using namespace OglImageAPI_NameSpace;
void getExtrinsicMatrix_Orthogonal(Mat& imgG,
                                   Mat& lms,
                                   mat4& model,
                                   string& errReason);

//get the first mac address, usuaully it's the ethernet address
bool getMacAddress(char * macAddr) {
    
    struct ifreq ifr;
    struct ifconf ifc;
    char buf[1024];
    int success = 0;

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1) { /* handle error*/ };

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) { /* handle error */ }

    struct ifreq* it = ifc.ifc_req;
    const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

    for (; it != end; ++it) {
        strcpy(ifr.ifr_name, it->ifr_name);
        if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
                if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
                    success = 1;
                    break;
                }
            }
        }
        else { /* handle error */ }
    }

    if (success) {
        //memcpy(macAddr, ifr.ifr_hwaddr.sa_data, 6);
        sprintf(macAddr, "%02x%02x%02x%02x%02x%02x", 
                (unsigned char) ifr.ifr_addr.sa_data[0],
                (unsigned char) ifr.ifr_addr.sa_data[1],
                (unsigned char) ifr.ifr_addr.sa_data[2],
                (unsigned char) ifr.ifr_addr.sa_data[3],
                (unsigned char) ifr.ifr_addr.sa_data[4],
                (unsigned char) ifr.ifr_addr.sa_data[5]);
    }
    return success;
}

bool gIsValid = false;
OglImageAPI::OglImageAPI():models((rowsM + 1) * numModels, colsM, CV_32F), lms0(1, colsM, CV_32F)
{
    char mac_address[12]; //mac address string
    if( getMacAddress( mac_address ) ) {
        int i=0;
        while( SELECTED_MAC_ADDRESS[i] != NULL ) {
            if( !strcmp( SELECTED_MAC_ADDRESS[i], mac_address ) ) {
                gIsValid = true;
                break;
            }
            i++;
        }
    }
    if( gIsValid ) {
        //load the model
        loadModel("/usr/bin/Model/models.txt", (rowsM + 1) * numModels, colsM, models);
        kps0 = loadLandmarks("/usr/bin/Model/initlms.txt", numLMs);
        kps2mat(kps0, lms0, numLMs);
        face.load("/usr/bin/Model/haarcascade_frontalface_default.xml");
    }
}

int OglImageAPI::ProcessImage(string& faceImg, 
                              mat4& model,
                              string& errReason) {
    if( !gIsValid ) {
        //cout << "invalid mac"<<endl;
        return UNRECOGNIZED_MAC_ADDR;
    }
    Mat img = imread(faceImg, CV_LOAD_IMAGE_COLOR);
    if( img.data == NULL ) {
        //unsupported image format
        //cout<<"invalid image format"<<endl;
        return UNRECOGNIZED_IMAGE_FORMAT;
    }
    Mat imgG;
    cvtColor(img, imgG, CV_BGR2GRAY);
    
    vector<Rect> faceRegion;
    face.detectMultiScale(imgG, faceRegion, 1.1, 3, CV_HAAR_FIND_BIGGEST_OBJECT|CV_HAAR_SCALE_IMAGE, Size(30,30));
    if (faceRegion.size() != 1) {
        //cout << "ERROR: incorrect number of faces detected" << endl;
        return UNRECOGNIZED_FACE_NUMBER;
    }

    Mat lmsT = lms0.clone();
    vector<KeyPoint> kpsT = kps0;
    transTemplate(lmsT, numLMs, faceRegion[0].x + faceRegion[0].width/2 - lms0.at<float>(16), faceRegion[0].y + faceRegion[0].height - lms0.at<float>(17));
    float xScale = 0.8*faceRegion[0].width/(lmsT.at<float>(32) - lmsT.at<float>(0));
    for (int i = 0; i < colsM; i++) {
        if (i%2 == 0) lmsT.at<float>(i) = xScale*(lmsT.at<float>(i)-lmsT.at<float>(16)) + lmsT.at<float>(16);
        else lmsT.at<float>(i) = lmsT.at<float>(17)-xScale*(lmsT.at<float>(17)-lmsT.at<float>(i));
    }
    mat2kps(lmsT, kpsT, numLMs);
    Mat lms = lmsT;
    vector<KeyPoint> kps = kpsT;
    
    for (int k = 0; k < numIter; k++) {
        for (int i = 0; i < numModels; i++) {
            extractor.compute(imgG, kps, descriptor);
            Mat des1D = descriptor.reshape(0,1);
            des1D = des1D.t();
            des1D.resize(rowsM+1, Scalar(1));
            des1D = des1D.t();
            lms = lms + des1D*models(Range(i*(rowsM+1)+0,(i+1)*(rowsM+1)),Range::all());
            mat2kps(lms, kps, numLMs);
        }
    }

    getExtrinsicMatrix_Orthogonal(img, lms, model, errReason);

    return NO_ERROR;
}

void getExtrinsicMatrix_Orthogonal(Mat &img,
                                   Mat &lms,
                                   mat4& model,
				   string &errMsg) {
    
    vector<Point2f> landmarks = mat2Point2f(lms);
    float rmRatio = estimateRM(landmarks);

    Mat rotAxis, newNormal;
    float rotAngle, rotAngle_newNormal;
    poseEstimate_3D(img, rmRatio, landmarks, rotAxis, rotAngle, newNormal, rotAngle_newNormal);
    
    string vertexFile = "/usr/bin/Model/facemodel/vertexlist_113.wfm";
    string faceFile = "/usr/bin/Model/facemodel/facelist_184.wfm";
    vector<myvec3> verticesM = readVertices(vertexFile);
    vector<triangle> faces = readFaces(faceFile);
    model = genRT(img, verticesM, faces, rmRatio, landmarks, rotAxis, rotAngle, newNormal, rotAngle_newNormal);
}

namespace OglImageAPI_NameSpace {
glm::mat4 genRT(Mat& img, vector<myvec3> &verticesM, vector<triangle> &faces, float rmRatio, vector<Point2f> &landmarks, Mat &rotAxis, float &rotAngle, Mat &newNormal, float &rotAngle_newNormal){
    Point2f le = landmarks[36];
    Point2f re = landmarks[45];
    Point2f lm = landmarks[48];
    Point2f rm = landmarks[54];
    Point2f ntip = landmarks[30];
    Point2f me = 0.5*(le+re);
    Point2f mm = 0.5*(lm+rm);
    Point2f nbase = mm + rmRatio*(me-mm);
    
    vector<myvec3> verticesMTmp(verticesM);
    myrotate(verticesMTmp, rotAngle_newNormal, 0, 0, 1);
    myrotate(verticesMTmp, rotAngle, rotAxis.at<float>(0,0), rotAxis.at<float>(0,1), rotAxis.at<float>(0,2));
    Point2f re_mesh(verticesMTmp[faces[144].a].x, verticesMTmp[faces[144].a].y);
    Point2f le_mesh(verticesMTmp[faces[114].a].x, verticesMTmp[faces[114].a].y);
    Point2f rm_mesh(verticesMTmp[faces[78].b].x, verticesMTmp[faces[78].b].y);
    Point2f lm_mesh(verticesMTmp[faces[72].c].x, verticesMTmp[faces[72].c].y);
    float len_mesh = sqrt((re_mesh.x-le_mesh.x)*(re_mesh.x-le_mesh.x) + (re_mesh.y-le_mesh.y)*(re_mesh.y-le_mesh.y));
    float len_img = sqrt((re.x-le.x)*(re.x-le.x) + (re.y-le.y)*(re.y-le.y));
    float scaleG = len_img/len_mesh;
    
    Point2f ntip_mesh(verticesM[faces[55].b].x, verticesM[faces[55].b].y);
    Point2f me_mesh = 0.5*(re_mesh+le_mesh);
    float ntip2eye_mesh = sqrt((me_mesh.x-ntip_mesh.x)*(me_mesh.x-ntip_mesh.x) + (me_mesh.y-ntip_mesh.y)*(me_mesh.y-ntip_mesh.y));
    float ntip2eye_img = sqrt((me.x-ntip.x)*(me.x-ntip.x) + (me.y-ntip.y)*(me.y-ntip.y));
    float ratioY = ntip2eye_img/ntip2eye_mesh;
    
    myscale(verticesM, scaleG, ratioY, scaleG);
    
    float k1_1 = 0;
    float k1_2 = 0;
    float k1_3 = 1;
    float k2_1 = rotAxis.at<float>(0,0);
    float k2_2 = rotAxis.at<float>(0,1);
    float k2_3 = rotAxis.at<float>(0,2);
    Mat K1 = (Mat_<float>(3,3) << 0, -k1_3, k1_2, k1_3, 0, -k1_1, -k1_2, k1_1, 0);
    Mat K2 = (Mat_<float>(3,3) << 0, -k2_3, k2_2, k2_3, 0, -k2_1, -k2_2, k2_1, 0);
    Mat IMat = Mat::eye(3, 3, CV_32F);
    Mat rotMat1 = IMat + sin(rotAngle_newNormal*PI/180)*K1 + (1-cos(rotAngle_newNormal*PI/180))*K1*K1;
    Mat rotMat2 = IMat + sin(rotAngle*PI/180)*K2 + (1-cos(rotAngle*PI/180))*K2*K2;

    myrotate(verticesM, rotAngle_newNormal, 0, 0, 1);
    myrotate(verticesM, rotAngle, rotAxis.at<float>(0,0), rotAxis.at<float>(0,1), rotAxis.at<float>(0,2));

    int w = img.cols;
    int h = img.rows;
    float asp = float(w)/h;

    float xShift = (2*asp*ntip.x/w-asp) - 2*asp*verticesM[faces[55].b].x/w;
    float yShift = (2*(1-ntip.y/h)-1.0) - 2*(-verticesM[faces[55].b].y/h);   

    float scaleGX = 2*scaleG/h;
    ratioY = 2*ratioY/h;
    /*
    printf("xShift=%.2f\n", xShift);
    printf("yShift=%.2f\n", yShift);
    printf("-rotAngle_newNormal=%.2f\n", -rotAngle_newNormal);
    printf("-rotAngle=%.2f\n", -rotAngle);
    printf("scaleGX=%.2f\n", scaleGX);
    printf("scaleG=%.2f\n", scaleG);
    printf("ratioY=%.2f\n", ratioY);
    printf("rotAxis.at<float>(0,0)=%.2f\n", rotAxis.at<float>(0,0));
    printf("rotAxis.at<float>(0,1)=%.2f\n", rotAxis.at<float>(0,1));
    printf("rotAxis.at<float>(0,2)=%.2f\n", rotAxis.at<float>(0,2));
    */
    glm::mat4 model;
    model = glm::translate(model, glm::vec3(xShift, yShift, 0.0f));
    model = glm::rotate(model, -radians(rotAngle_newNormal), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::rotate(model, -radians(rotAngle), glm::vec3(rotAxis.at<float>(0,0), rotAxis.at<float>(0,1), rotAxis.at<float>(0,2)));
    model = glm::translate(model, glm::vec3(0.0f, ratioY*0.35f, scaleG*0.13f));
    model = glm::scale(model, glm::vec3(scaleGX*1.0f, scaleGX*1.0f, scaleGX*1.0f));
    
    return model;
}

float estimateRM(vector<Point2f> &landmarks){
    
    Point2f le = landmarks[36];
    Point2f re = landmarks[45];
    Point2f lm = landmarks[48];
    Point2f rm = landmarks[54];
    Point2f ntip = landmarks[30];
    
    Point2f me = 0.5*(le+re);
    Point2f mm = 0.5*(lm+rm);
    
    Point2f ptGlass;
    Point2f ptEar;
    
    float disLeft = (landmarks[0].x - landmarks[36].x)*(landmarks[0].x - landmarks[36].x) + (landmarks[0].y - landmarks[36].y)*(landmarks[0].y - landmarks[36].y);
    float disRight = (landmarks[16].x - landmarks[45].x)*(landmarks[16].x - landmarks[45].x) + (landmarks[16].y - landmarks[45].y)*(landmarks[16].y - landmarks[45].y);
    
    float eyeDis = (landmarks[36].x - landmarks[45].x)*(landmarks[36].x - landmarks[45].x) + (landmarks[36].y - landmarks[45].y)*(landmarks[36].y - landmarks[45].y);
    
    if (disLeft > disRight) {
        float tmpX = (landmarks[36].x - landmarks[45].x)*RG + landmarks[45].x;
        float tmpY = (landmarks[36].y - landmarks[45].y)*RG + landmarks[45].y;
        ptGlass = Point2f(tmpX, tmpY);
        ptEar = Point2f(landmarks[0].x, landmarks[0].y);
    }else{
        float tmpX = (landmarks[45].x - landmarks[36].x)*RG + landmarks[36].x;
        float tmpY = (landmarks[45].y - landmarks[36].y)*RG + landmarks[36].y;
        ptGlass = Point2f(tmpX, tmpY);
        ptEar = Point2f(landmarks[16].x, landmarks[16].y);
    }
    
    float x1 = me.x;
    float y1 = me.y;
    float x2 = mm.x;
    float y2 = mm.y;
    float x3 = ntip.x;
    float y3 = ntip.y;
    float x4 = x3-10;
    float y4 = (x4-x3)*(ptGlass.y-ptEar.y)/(ptGlass.x-ptEar.x) + y3;
    float x = ((x1*y2-y1*x2)*(x3-x4) - (x1-x2)*(x3*y4-x4*y3))/((x1-x2)*(y3-y4)-(y1-y2)*(x3-x4));
    float y = ((x1*y2-y1*x2)*(y3-y4) - (y1-y2)*(x3*y4-x4*y3))/((x1-x2)*(y3-y4)-(y1-y2)*(x3-x4));
    float rmRatio = sqrt((x-x2)*(x-x2)+(y-y2)*(y-y2))/sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
    
    return rmRatio;
    
}

void poseEstimate_3D(Mat img, float rmRatio, vector<Point2f> &landmarks, Mat &rotAxis, float &rotAngle, Mat &newNormal, float &rotAngle_newNormal){
    
    Point2f le = landmarks[36];
    Point2f re = landmarks[45];
    Point2f lm = landmarks[48];
    Point2f rm = landmarks[54];
    Point2f ntip = landmarks[30];
    Point2f me = 0.5*(le+re);
    Point2f mm = 0.5*(lm+rm);
    Point2f nbase = mm + rmRatio*(me-mm);
    
    float tau_deg = angleCalc(ntip-nbase, Point2f(1,0));
    float theta_deg = angleCalc(ntip-nbase, me-nbase);
    Point2f tmp = ntip-nbase;
    if (tmp.y < 0) {
        tau_deg = 360 - tau_deg;
    }
    
    float ln = sqrt((ntip.x-nbase.x)*(ntip.x-nbase.x) + (ntip.y-nbase.y)*(ntip.y-nbase.y));
    float lf = sqrt((me.x-mm.x)*(me.x-mm.x) + (me.y-mm.y)*(me.y-mm.y));
    float m1 = (ln*ln/(lf*lf));
    float m2 = cos(theta_deg/180*PI)*cos(theta_deg/180*PI);
    float sigma_rad = 0;
    float sigma_deg = 0;
    if (m2 == 1) sigma_rad = acos(sqrt(RN*RN/(RN*RN+m1)));
    else{
        sigma_rad = acos(sqrt((RN*RN-m1-2*m2*RN*RN+sqrt((m1-RN*RN)*(m1-RN*RN)+4*m1*m2*RN*RN))/(2*(1-m2)*RN*RN)));
    }
    sigma_deg = sigma_rad/PI*180;
    
    Mat refNormal = (Mat_<float>(1,3) << 0,0,-1);
    newNormal = (Mat_<float>(1,3) << sin(sigma_deg/180*PI)*cos(tau_deg/180*PI), sin(sigma_deg/180*PI)*sin(tau_deg/180*PI), cos(sigma_deg/180*PI));
    
    rotAxis = refNormal.cross(newNormal);
    rotAxis = rotAxis/norm(rotAxis,NORM_L2);
    rotAngle = sigma_deg;
    
    rotAngle_newNormal = angleCalc(re-le, Point2f(1,0));
    Point2f tmp1 = re - le;
    if (tmp1.y < 0) {
        rotAngle_newNormal = 360 - rotAngle_newNormal;
    }
}

float angleCalc(Point2f pt1, Point2f pt2){
    float len1 = sqrt(pt1.x*pt1.x + pt1.y*pt1.y);
    float len2 = sqrt(pt2.x*pt2.x + pt2.y*pt2.y);
    
    float dot = pt1.x*pt2.x + pt1.y*pt2.y;
    float a = dot/(len1*len2);
    
    if (a >= 1.0) {
        return 0;
    }else if (a <= -1.0){
        return 180;
    }else{
        return acos(a)/PI*180;
    }
    
}

void PMat2P(Mat& PMat, float P[]){
    P[3] = PMat.at<float>(0,3);
    P[4] = PMat.at<float>(1,3);
    P[5] = PMat.at<float>(2,3);
    
    P[2] = atan(-PMat.at<float>(1,0)/PMat.at<float>(0,0));
    P[0] = atan(-PMat.at<float>(2,1)/PMat.at<float>(2,2));
    P[1] = atan(PMat.at<float>(2,0)/sqrt(PMat.at<float>(2,1)*PMat.at<float>(2,1) + PMat.at<float>(2,2)*PMat.at<float>(2,2)));
}



void myscale(vector<myvec3> & vertices, float sx, float sy, float sz)
{
    for (int i = 0; i < vertices.size(); i++){
        vertices[i].x *= sx;
        vertices[i].y *= sy;
        vertices[i].z *= sz;
    }
    
}

void mytransform(vector<myvec3> & vertices, float P[])
{
    myrotateXYZ(vertices,P[0]*180,P[1]*180,P[2]*180);
    mytranslate(vertices, P[3], P[4], P[5]);
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

void myrotateWithMat(vector<myvec3> & vertices, Mat rotMat) // Rotate at origin point
{
    for (int i = 0; i < vertices.size(); i++){
        float vx = vertices[i].x;
        float vy = vertices[i].y;
        float vz = vertices[i].z;
        Mat vMat = (Mat_<float>(3,1) << vx, vy, vz);
        Mat vN = rotMat * vMat;
        
        vertices[i].x = vN.at<float>(0,0);
        vertices[i].y = vN.at<float>(1,0);
        vertices[i].z = vN.at<float>(2,0);
    }
}

void myrotate(vector<myvec3> & vertices, float angle, float ux, float uy, float uz) // Rotate at origin point
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

void mytranslate(vector<myvec3> & vertices, float tx, float ty, float tz)
{
    for (int i = 0; i < vertices.size(); i++){
        vertices[i].x += tx;
        vertices[i].y += ty;
        vertices[i].z += tz;
    }
    
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
        vertex.x = - vertex.x;
        //vertex.y = -vertex.y - 0.17;
        //vertex.z = -vertex.z;
        vertex.y = - vertex.y - 0.222; //faces[55].b
        //vertex.z = -vertex.z - 0.1827;
        vertex.z = - vertex.z - 0.05;
        vertices.push_back(vertex);
        //cout << vertex.x << " " << vertex.y << " " << vertex.z << endl;
    }
    
    ifs.close();
    
    return vertices;
}

vector<Point2f> mat2Point2f(Mat lms){
    vector<Point2f> res;
    for (int i = 0; i < NUMLMS; i++) {
        res.push_back(Point2f(lms.at<float>(0,i*2), lms.at<float>(0,i*2+1)));
    }
    
    return res;
}
}

//original functions
void drawPoints(Mat image, vector<KeyPoint> kps, Scalar color){
    for (unsigned int i = 0; i < kps.size(); i++){
        Point center = kps[i].pt;
        circle(image, center, 2.0, color, -1, 8);
    }
}

void transTemplate(Mat lms, int numLMs, float shiftX, float shiftY){
    for (int i = 0; i < numLMs; i++){
        lms.at<float>(2*i) += shiftX;
        lms.at<float>(2*i+1) += shiftY;
    }
}

void mat2kps(Mat lms, vector<KeyPoint> &kps, int numLMs){
    kps.clear();
    for (int i = 0; i < numLMs; i++){
        kps.push_back(KeyPoint(lms.at<float>(2*i), lms.at<float>(2*i+1), KPSIZE));
    }
}

void kps2mat(vector<KeyPoint> &kps, Mat lms, int numLMs){
    for (int i = 0; i < numLMs; i++){
        lms.at<float>(2*i) = kps[i].pt.x;
        lms.at<float>(2*i+1) = kps[i].pt.y;
    }
}

vector<KeyPoint> loadLandmarks(const char * lmFile, int numLM){
    fstream fin;
    fin.open(lmFile, fstream::in);
    
    vector<KeyPoint> landmarks;
    float xData, yData;
    for (int i = 0; i < numLM; i++){
        fin >> xData >> yData;
        landmarks.push_back(KeyPoint(xData, yData, KPSIZE));
    }
    fin.close();
    return landmarks;
}

void loadModel(const char * modelFile, int rows, int cols, Mat data){
    fstream fin;
    fin.open(modelFile, fstream::in);
    
    for (int i = 0; i < rows; i ++){
        for (int j = 0; j < cols; j++){
            fin >> data.at<float>(i,j);
        }
    }
    
    fin.close();
}
