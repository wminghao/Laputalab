#include <iostream>
#include <vector>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"

#include "main.h"
#include "utilities.h"
#include "objloader.hpp"

// GLFW
#include <GLFW/glfw3.h>
#include "glasses.h"

using namespace std;
using namespace cv;

#define REALTIME 1
#define GLASSON 1

//Facial Model Source File
string vertexFile = "/Users/howard/AR_lib/LaputaDesktop2/VET/facemodel/vertexlist_113.wfm";
string faceFile = "/Users/howard/AR_lib/LaputaDesktop2/VET/facemodel/facelist_184.wfm";
const char* glassesFile = "/Users/howard/AR/LaputaApp/Resources/3dmodels/3dGlasses/RanGlass.obj"; //"/Users/howard/AR_lib/LaputaDesktop2/VET/Glasses.obj";//
const char* glassesVsh = "/Users/howard/AR_lib/LaputaDesktop/LaputaDesktop/3dGlasses/3dGlassesVertexShaderGL.vsh";
const char* glassesFsh = "/Users/howard/AR_lib/LaputaDesktop/LaputaDesktop/3dGlasses/3dGlassesFragmentShaderGL.fsh";
const char* fragName = "outFrag";

string videoFile = "./demo1.mov";

int calibrated = false;
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (  action == GLFW_PRESS ) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        } else {
            calibrated = !calibrated;
        }
    }
}

mat4 initMat;
mat4 initRotTransMat;

static Mat genMVPMat3x4(Glasses& glasses, float P_ar[]) {
    Mat curRotTransMat = P2PMat(P_ar);
    glm::mat4 curRotTransMat4;
    mat3x4ToMat4(&curRotTransMat, curRotTransMat4);
    
    Mat curCVMat(3, 4, DataType<float>::type);
    glm::mat4 curMat = glasses.getMat(curRotTransMat4);
    mat4ToMat3x4(curMat, &curCVMat);
    return curCVMat;
}

static void drawOpenGLGlasses(GLuint& dstTexture, Mat& frameOrig, Glasses& glasses, GLFWwindow* window, mat4& curRotTransMat ) {
    //TODO, no need in the future
    Mat frame = frameOrig.clone();
    
    ///////////////////////////////////////////////////////////////////////////////////
    //glw window draw
    //NEXT convert from mat to opengl texture
    //http://stackoverflow.com/questions/16809833/opencv-image-loading-for-opengl-texture
    ///////////////////////////////////////////////////////////////////////////////////
    glBindTexture(GL_TEXTURE_2D, dstTexture);
    flip(frame, frame, 0); //flip x
    glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                 0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                 GL_RGBA,           // Internal colour format to convert to
                 frame.cols,        // Image width  i.e. 640 for Kinect in standard mode
                 frame.rows,        // Image height i.e. 480 for Kinect in standard mode
                 0,                 // Border width in pixels (can either be 1 or 0)
                 GL_BGR,            // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                 GL_UNSIGNED_BYTE,  // Image data type
                 frame.ptr()); // The actual image data itself
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //set new matrix
    glasses.setRotTransMat(curRotTransMat);
    
    //draw glasses
    glasses.render(dstTexture, dstTexture);
    
    // Swap the buffers
    glfwSwapBuffers(window);
    
    // Check and call events
    glfwPollEvents();
}

int main()
{
    //src, 4:3
    const int srcWidth = 640;
    const int srcHeight = 480;
    
    ////////////////
    //glw window
    ////////////////
    // Init GLFW
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 32); // 32x antialiasing, very aggressive
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    
    GLFWwindow* window = glfwCreateWindow(srcWidth, srcHeight, "OpenGL Mirror ", nullptr, nullptr); // Windowed
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);
    
    ////////////////
    //opengl objects
    ////////////////
    // add error checking here
    GLuint dstTexture;
    glGenTextures(1, &dstTexture);
    Glasses glasses(srcWidth, srcHeight);
    
    glasses.init(glassesVsh,
                 glassesFsh,
                 fragName,
                 glassesFile,
                 faceFile.c_str(),
                 vertexFile.c_str(),
                 0, ASPECT_RATIO_4_3);
    glasses.getInitMat(initMat, initRotTransMat);
    
    Mat initCVMat(3, 4, DataType<float>::type);
    mat4ToMat3x4(initMat, &initCVMat);
    cout<<"initCVMat: "<<initCVMat<<endl;
    
    Mat initRotTransCVMat(3, 4, DataType<float>::type);
    mat4ToMat3x4(initRotTransMat, &initRotTransCVMat);
    cout<<"initRotTransCVMat: "<<initRotTransCVMat<<endl;
    
    //------------Initialization Begin---------------
    
#if 0
    //Scales of candide3
    float xScale = V_WIDTH/4-10;//V_WIDTH/4+10; //V_WIDTH/4-10;
    float yScale = V_HEIGHT/4+20;//V_HEIGHT/4+25; //V_HEIGHT/4+20;
    float zScale = (xScale+yScale)/2;
#endif
    
    //Shape Factors
    float shapeFactor[SHAPEUNITS] = //for Xz
    {0.0f,0.5f,0.4f, //headheight, eyebrows vertical pos, eyes vertical pos;
        0.1f,0.0f,0.3f, // eyes width, eyes height, eye seperation dis;
        0.0f,1.0f,0.5f, // cheeks z, nose-z extension, nose vertical pos;
        0.0f,0.0f,0.3f, // nose pointing up, mouth vertical pos, mouth width;
        0.0f,0.0f}; // eyes vertical diff, chin width;

    float P[NP] =      //Position glasses and candide3 to the center of the window.
    {
        0.0f, 0.0f, 0.0f,   //normalized rotation parameter
        0.5f, 0.5f, 1.0f,  // normalized translation parameter
    };
    PMat2P(initRotTransCVMat, P); //convert from glasses matrix into P.
    
    
    float Ptest[NP] = {
        PI*1/6, PI*1/3, PI*1/4, 1,40,20
    };
    Mat ptestMat(3, 4, DataType<float>::type);
    ptestMat = P2PMat(Ptest);
    cout << ptestMat;
    float Pres[NP];
    PMat2P(ptestMat, Pres);
    
    
    vector<myvec3> verticesFromModel = readVertices(vertexFile);
    vector<triangle> faces = readFaces(faceFile);
    vector<myvec3> sfVertices = createSFVertices(verticesFromModel); //normalize, i.e., 40*40
    
#if 0
    vector<myvec3> verticesGlass; //glasses
    vector<myvec3> verticesG_1st; //position to the center of the window for calibration
    
    if (GLASSON) {
        //Read glasses model
        bool res = loadOBJ("/Users/howard/AR_lib/LaputaDesktop2/VET/Glasses.obj", verticesGlass);
        myrotate(verticesGlass, 90, 0, 1, 0);
        myrotate(verticesGlass, 180, 0, 0, 1);
        myscale(verticesGlass, 35, 35, 35);
        mytranslate(verticesGlass, 0, 10, -15);
        
        verticesG_1st = verticesGlass;
        mytransform(verticesG_1st, P);
    }
    
    vector<myvec3> verticesAdjusted(verticesFromModel);
    adjustShape(shapeFactor, verticesAdjusted, xScale, yScale, zScale); //based on the face, now it's xingze
#endif
    vector<myvec3> verticesAdjusted;
    glasses.getCandide3Vertices(verticesAdjusted); //read adjusted coordinates directly from opengl.
    
    unsigned char map[SF_HEIGHT][SF_WIDTH];
    float G[SF_HEIGHT*SF_WIDTH];
    float diff[SF_HEIGHT*SF_WIDTH];
    float sfImgInit[SF_HEIGHT][SF_WIDTH];
    float sfImgRef[SF_HEIGHT][SF_WIDTH];
    float sfImgMean[SF_HEIGHT][SF_WIDTH];
    float sfImgVar[SF_HEIGHT][SF_WIDTH];
    float sfImgSquare[SF_HEIGHT][SF_WIDTH];
    float sfImgTmp[SF_HEIGHT][SF_WIDTH];
    float sfImgCur[SF_HEIGHT][SF_WIDTH];
    float sfImgNext[SF_HEIGHT][SF_WIDTH];
    float sfImgDiff[SF_HEIGHT][SF_WIDTH];
    for (int j = 0; j < SF_HEIGHT; j++)
        for (int i = 0; i < SF_WIDTH; i++){
            map[j][i] = 255;
            sfImgInit[j][i] = 0;
            sfImgRef[j][i] = 0;
            sfImgMean[j][i] = 0;
            sfImgVar[j][i] = 0;
            sfImgSquare[j][i] = 0;
            sfImgTmp[j][i] = 0;
            sfImgCur[j][i] = 0;
            sfImgNext[j][i] = 0;
            sfImgDiff[j][i] = 0;
            G[j*SF_WIDTH + i] = 0;
            diff[j*SF_WIDTH + i] = 0;
        }
    
    float dMap[V_HEIGHT][V_WIDTH];
    for (int j = 0; j < V_HEIGHT; j++) {
        for (int i = 0; i < V_WIDTH; i++){
            dMap[j][i] = 10000;
        }
    }
    //dMap not useful for opengl
    
    //convert from normalized vertices and faces into map.
    createV2FMap(sfVertices, faces, map);
    
    //------------Initialization End---------------

    //------------   Tracking Begin ---------------
#if 0
    //Create tracking window
    namedWindow("Magic Mirror", 1);
    moveWindow("Magic Mirror", 400, 100);
    
    vector<myvec3> vertices_1st(verticesAdjusted);
    mytransform(vertices_1st, P); // Initialized vertices array
    //mytransformWithAU(vertices_1st, P, AU);
#endif
    
    VideoCapture cap;
    if (REALTIME) {
        cap.open(0);
    }else{
        cap.open(videoFile);
    }
    //VideoCapture cap(videoFile); // open the default video
    //VideoCapture cap(0);
    cap.set(CV_CAP_PROP_FRAME_WIDTH,640);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,480);
    if(!cap.isOpened())  // check if we succeeded
        return -1;
    
    int iF = 0;
    int iM = 0;
    int trackFlag = 0;
    int maxIter = 400;
    float sigma = 0.1;
    float C = 100;
    float errDiff = 1;
    float errTh = 0;
    float errTh1 = 2000; //10; //before first 40
    float errTh2 = 2000; //0.6; //after first 40
    float coe = 0;
    int K = 2;
    
    int errFlag = -1;
    
    float P_cur[NP];
    float P_tmp[NP];
    float P_next[NP];
    float res[NP];
    
    for (int i = 0; i < NP; i++){
        P_cur[i] = P[i];
        P_tmp[i] = P[i];
        P_next[i] = P[i];
        res[i] = 0;
    }
    
    float err_cur = 0;
    float err_next = 0;
#if 0
    vector<myvec3> vertices_last(vertices_1st);
#endif
    Mat frame, frame_tmp, image; // "image" to be renamed as "frameG"
    
    Mat frameOpenGL;
    
    while ( !glfwWindowShouldClose(window) )
    {
        cap >> frame;
        iF++;
        
        flip(frame, frame, 1);
        
        if (REALTIME) {
            if (trackFlag == 0){
                
                drawOpenGLGlasses(dstTexture, frame, glasses, window, initRotTransMat );
                
            #if 0
                frame_tmp = frame.clone();
                //drawMeshOnImg_Per(frame_tmp, faces, vertices_1st);
                drawGlassOnImg_Per(frame_tmp, verticesG_1st);
                imshow("Magic Mirror", frame_tmp);
                //waitKey(0);

                if ( waitKey(1) > 0 ) {
            #endif
                if ( calibrated ) {
                    calibrated = false;
                    
                    cvtColor(frame, image, CV_BGR2GRAY);
                    //tell the scaling factor of candide3, 3d mapping of vertices = vertices_1st and matrix. 3*4 matrix
                    //output sfImgRef.
                #if 0
                    genSFImg(image, faces, vertices_1st, sfVertices, map, sfImgRef);
                #endif
                    
                    genSFImgGL(image, faces, verticesAdjusted, sfVertices, map, sfImgRef, initCVMat);
                    //drawSFImg("SFImgRef", 400, 200, sfImgRef);
                    
                    for (int jY = 0; jY < SF_HEIGHT; jY++)
                        for (int jX = 0; jX < SF_WIDTH; jX++){
                            sfImgInit[jY][jX] = sfImgRef[jY][jX];
                        }
                    trackFlag = 1;
                }
                continue;
            }
            cvtColor(frame, image, CV_BGR2GRAY);
        } else {
            cvtColor(frame, image, CV_BGR2GRAY);
            if (iF == 1) {
#if 0
                genSFImg(image, faces, vertices_1st, sfVertices, map, sfImgRef);
#endif
                genSFImgGL(image, faces, verticesAdjusted, sfVertices, map, sfImgRef, initCVMat);
                for (int jY = 0; jY < SF_HEIGHT; jY++) {
                    for (int jX = 0; jX < SF_WIDTH; jX++){
                        sfImgInit[jY][jX] = sfImgRef[jY][jX];
                    }
                }
                continue;
            }
            
        }
        
        int curIter = 0;
        float rou = 1;
        
    #if 0
        vector<myvec3> vertices_cur(vertices_last);
        genSFImg(image, faces, vertices_cur, sfVertices, map, sfImgCur);
    #endif
        Mat curCVMat4 = genMVPMat3x4(glasses, P_cur);
        cout<<curCVMat4<<endl;
        genSFImgGL(image, faces, verticesAdjusted, sfVertices, map, sfImgCur, curCVMat4);
        drawSFImg("SFImgCur", 600, 200, sfImgCur);
        
        //drawSFImg("sfImgCur", 400, 100, sfImgCur);
        
        for (int j = 0; j < SF_HEIGHT; j++)
            for (int i = 0; i < SF_WIDTH; i++){
                sfImgDiff[j][i] = sfImgCur[j][i] -  sfImgRef[j][i];
            }
        err_cur = errorCalc(sfImgDiff);
        
        if (err_cur > 400) {
            //imshow("Magic Mirror", frame);
            
            for (int jY = 0; jY < SF_HEIGHT; jY++)
                for (int jX = 0; jX < SF_WIDTH; jX++){
                    sfImgRef[jY][jX] = sfImgInit[jY][jX];
                }
        #if 0
            vertices_last = vertices_1st;
        #endif
            for (int i = 0; i < NP; i++){
                P_cur[i] = P[i];
                P_tmp[i] = P[i];
            }
            continue;
        }
        
        while (++ curIter < maxIter) {
            //cout << curIter << endl;
            for (int i = 0; i < NP; i++){
                for (int jY = 0; jY < SF_HEIGHT; jY++)
                    for (int jX = 0; jX < SF_WIDTH; jX++){
                        G[jY*SF_WIDTH+jX] = 0;
                    }
                for (int k = -K/2; k <= K/2; k++){
                    if (k == 0) continue;
                    P_tmp[i] += k*sigma;
                #if 0
                    vector<myvec3> vertices_tmp(verticesAdjusted);
                    mytransform(vertices_tmp, P_tmp);
                    //mytransformWithAU(vertices_tmp, P_tmp, AU);
                    genSFImg(image, faces, vertices_tmp, sfVertices, map, sfImgTmp);
                #endif

                    Mat tmpCVMat = genMVPMat3x4(glasses, P_tmp);
                    genSFImgGL(image, faces, verticesAdjusted, sfVertices, map, sfImgTmp, tmpCVMat);
                    drawSFImg("sfImgTmp", 800, 200, sfImgTmp);
                    //waitKey(0);
                    
                    
                    for (int jY = 0; jY < SF_HEIGHT; jY++)
                        for (int jX = 0; jX < SF_WIDTH; jX++){
                            G[jY*SF_WIDTH+jX] += (1.0/K)*(sfImgTmp[jY][jX]-sfImgCur[jY][jX])/(k*sigma);
                        }
                    P_tmp[i] -= k*sigma;
                }
                
                Mat GM = Mat(SF_HEIGHT*SF_WIDTH, 1, CV_32FC(1), G);
                Mat GPM = Mat(SF_HEIGHT*SF_WIDTH, 1, CV_32FC(1), G);
                invert(GM, GPM, DECOMP_SVD);
                //cout<<GPM<<endl;
                
                for (int j = 0; j < SF_HEIGHT; j++)
                    for (int i = 0; i < SF_WIDTH; i++){
                        if (map[j][i] != 255){
                            float d = sfImgDiff[j][i];
                            diff[j*SF_WIDTH + i] = d;
                            }
                        }
                
                Mat diffM = Mat(SF_HEIGHT*SF_WIDTH, 1, CV_32FC(1), diff);
                Mat resM = GPM*diffM;
                
                res[i] = resM.at<float>(0,0);
                
                //cout << res[i] << endl;
                
                P_next[i] = P_cur[i] - rou*res[i];
                
            }
            
            
#if 0
            vector<myvec3> vertices_next(verticesAdjusted);
            mytransform(vertices_next, P_next);
            //mytransformWithAU(vertices_next, P_next, AU);
            genSFImg(image, faces, vertices_next, sfVertices, map, sfImgNext);
#endif
            Mat nextCVMat = genMVPMat3x4(glasses, P_next);
            genSFImgGL(image, faces, verticesAdjusted, sfVertices, map, sfImgNext, nextCVMat);
            
            for (int j = 0; j < SF_HEIGHT; j++)
                for (int i = 0; i < SF_WIDTH; i++){
                    sfImgDiff[j][i] = sfImgNext[j][i] -  sfImgRef[j][i];
                }
            err_next = errorCalc(sfImgDiff);
            
            cout << "Err_cur: " << err_cur << endl;
            cout << "Err_next: " << err_next << endl;
            
            if (abs(err_next - err_cur) < errDiff) {
                //drawSFImg("sfImgRef", 400+SF_WIDTH, 100, sfImgRef);
                //drawSFImg("sfImgNext", 400+2*SF_WIDTH, 100, sfImgNext);
                break;
            }else if (err_next < err_cur){
                for (int i = 0; i < NP; i++){
                    P_cur[i] = P_next[i];
                    P_tmp[i] = P_next[i];
                }
                //vertices_cur = vertices_next;
                err_cur = err_next;
                continue;
            }else{
                while (err_next > err_cur) {
                    rou = rou/2;
                    if (rou < 0.0001) { //0.0001
                        break;
                    }
                    //cout << "Rou: " << rou << endl;
                    for (int i = 0; i < NP; i++){
                        P_next[i] = P_cur[i] - rou*res[i];
                    }
                #if 0
                    vector<myvec3> vertices_tmp1(verticesAdjusted);
                    mytransform(vertices_tmp1, P_next);
                    //mytransformWithAU(vertices_tmp1, P_next, AU);
                    genSFImg(image, faces, vertices_tmp1, sfVertices, map, sfImgNext);
                #endif
                    
                    Mat nextCVMat = genMVPMat3x4(glasses, P_next);
                    genSFImgGL(image, faces, verticesAdjusted, sfVertices, map, sfImgNext, nextCVMat);
                    
                    for (int j = 0; j < SF_HEIGHT; j++)
                        for (int i = 0; i < SF_WIDTH; i++){
                            sfImgDiff[j][i] = sfImgNext[j][i] -  sfImgRef[j][i];
                        }
                    err_next = errorCalc(sfImgDiff);
                }
                
                for (int i = 0; i < NP; i++){
                    P_cur[i] = P_next[i];
                    P_tmp[i] = P_next[i];
                }
                
                err_cur = err_next;
                
                continue;
                
            }
            
        }
    #if 0
        vector<myvec3> vertices_final(verticesAdjusted);
        mytransform(vertices_final, P_next);
    #endif
        //mytransformWithAU(vertices_final, P_next, AU);
        
        // By far, coe = 0 works better
        
        if (errFlag < 0) errTh = errTh1;
        else
            errTh = errTh2;
        
        cout << "Err_cur: " << err_cur << endl;
        if (err_cur <= errTh) {
            iM++;
            //changes start
            //cout << "Err_cur: " << err_cur << endl;
            //cout << "ErrTh: " << errTh << endl;
            //cout << (P_next[0]*P_next[0] + P_next[1]*P_next[1]) << endl;
            if ((P_next[0]*P_next[0] + P_next[1]*P_next[1]) > 1.0/500) {
                for (int jY = 0; jY < SF_HEIGHT; jY++)
                    for (int jX = 0; jX < SF_WIDTH; jX++){
                        sfImgRef[jY][jX] = (coe*sfImgRef[jY][jX]*iM + sfImgNext[jY][jX])/(coe*iM+1);
                        sfImgMean[jY][jX] = (sfImgMean[jY][jX]*iM + sfImgNext[jY][jX])/(iM + 1);
                        sfImgSquare[jY][jX] = sfImgSquare[jY][jX] + pow(sfImgNext[jY][jX],2);
                        sfImgVar[jY][jX] = (sfImgSquare[jY][jX] - pow((sfImgMean[jY][jX]*iM),2)/(iM + 1))/iM; //Biased or Unbiased?
                    }
                errFlag = 1;

            }else{
                for (int jY = 0; jY < SF_HEIGHT; jY++)
                    for (int jX = 0; jX < SF_WIDTH; jX++){
                        sfImgRef[jY][jX] = sfImgInit[jY][jX];
                    }
                errFlag = -1;
       
            }
            //changes end
            /*
            for (int jY = 0; jY < SF_HEIGHT; jY++)
                for (int jX = 0; jX < SF_WIDTH; jX++){
                    sfImgRef[jY][jX] = (coe*sfImgRef[jY][jX]*iM + sfImgNext[jY][jX])/(coe*iM+1);
                    sfImgMean[jY][jX] = (sfImgMean[jY][jX]*iM + sfImgNext[jY][jX])/(iM + 1);
                    sfImgSquare[jY][jX] = sfImgSquare[jY][jX] + pow(sfImgNext[jY][jX],2);
                    sfImgVar[jY][jX] = (sfImgSquare[jY][jX] - pow((sfImgMean[jY][jX]*iM),2)/(iM + 1))/iM; //Biased or Unbiased?
                }
            */
            
            
            //printPara(P_next);
            
            if (GLASSON) {
                
                Mat nextRotTransMat = P2PMat(P_next);
                glm::mat4 nextRotTransMat4;
                mat3x4ToMat4(&nextRotTransMat, nextRotTransMat4);
                //TODO
                nextRotTransMat4[3][2] = 0;
                drawOpenGLGlasses(dstTexture, frame, glasses, window, nextRotTransMat4 );
                
                
                Mat nextCVMat4 = genMVPMat3x4(glasses, P_next);
                drawMeshOnImgGL(image, faces, verticesAdjusted, nextCVMat4);
                imshow("Debuging", image);
                waitKey(1);
                
            #if 0
                vector<myvec3> verticesG(verticesGlass);
                createDepthMap(vertices_final, faces, dMap);
                mytransform(verticesG, P_next); //P_next is the 3*4 matrix.
                /*
                if (iF < 200) {
                    drawGlassOnImg_Per_Real_RGB(frame, verticesG, dMap, 20, 20, 20);
                }else if (iF < 300){
                    drawGlassOnImg_Per_Real_RGB(frame, verticesG, dMap, 102, 102, 253);
                }else if (iF < 400){
                    drawGlassOnImg_Per_Real_RGB(frame, verticesG, dMap, 0, 170, 85);
                }else{
                    drawGlassOnImg_Per_Real_RGB(frame, verticesG, dMap,20, 20, 20);
                }
                */
                
                //drawMeshOnImg_Per(frame, faces, vertices_final);
                
                drawGlassOnImg_Per_Real(frame, verticesG, dMap);
                imshow("Magic Mirror", frame);
                //waitKey(0);
            #endif
            } else {
                
            #if 0
                drawMeshOnImg_Per(frame, faces, vertices_final);
                //cout << "Waiting for the Next Frame ..." << endl;
                imshow("Magic Mirror", frame);
                //waitKey(0);
            #endif
            }
            
            
        #if 0
            vertices_last = vertices_final;
        #endif
            for (int i = 0; i < NP; i++){
                P_cur[i] = P_next[i];
                P_tmp[i] = P_next[i];
            }
            
            
        }else{
            //
            
            for (int jY = 0; jY < SF_HEIGHT; jY++)
                for (int jX = 0; jX < SF_WIDTH; jX++){
                    sfImgRef[jY][jX] = sfImgInit[jY][jX];
                }
            
            drawOpenGLGlasses(dstTexture, frame, glasses, window, initRotTransMat );
            
        #if 0
            //reset the size.
            //drawMeshOnImg_Per(frame, faces, vertices_1st);
            drawGlassOnImg_Per(frame, verticesG_1st);
            //waitKey(0);
            //cout << "Debuging ..." << endl;
            imshow("Magic Mirror", frame);
            //waitKey(0);
            vertices_last = vertices_1st;
        #endif
            
            for (int i = 0; i < NP; i++){
                P_cur[i] = P[i];
                P_tmp[i] = P[i];
            }
        }
        
        /*
        // Save to images
        ostringstream convert;
        convert << iF;
        imwrite(outFolder + "/frame-" + convert.str() + ".jpg", frame);
         */

        //if(waitKey(1) >= 0) break;
    }

    //------------   Tracking End  ---------------
    
    ////////////////////
    //glw window cleanup
    ////////////////////
    glDeleteTextures(1, &dstTexture);
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}