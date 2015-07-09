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

//jpeg compressor
#include "jpge.h"

//glasses
#include "glasses.h"

using namespace std;
using namespace cv;

#define REALTIME 1
#define GLASSON 0
#define SHOWTWOWIN 0
#define OPENGL_2_1 1

//Facial Model Source File
string vertexFile = pathPrefix + "demo/LaputaDesktop3/VET/facemodel/vertexlist_113.wfm";
string faceFile = pathPrefix + "demo/LaputaDesktop3/VET/facemodel/facelist_184.wfm";
const string glassesFile[] = { pathPrefix + "LaputaApp/Resources/3dmodels/3dGlasses/RanGlasses2.obj",
                               pathPrefix + "LaputaApp/Resources/3dmodels/3dGlasses/purpleglasses2.obj",
                               pathPrefix + "LaputaApp/Resources/3dmodels/3dGlasses/blackglasses2.obj"};
const char* fragName = "outFrag";

const string savedJpegFilePath = pathPrefix + "saved.jpg";
string videoFile = "./demo1.mov";


//src, 4:3
const int srcWidth = 640;
const int srcHeight = 480;
Glasses glasses(srcWidth, srcHeight);

int calibrated = false;
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (  action == GLFW_PRESS ) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        } else if( key == GLFW_KEY_1){
            glasses.reloadGlasses(glassesFile[0].c_str());
        } else if( key == GLFW_KEY_2){
            glasses.reloadGlasses(glassesFile[1].c_str());
        } else if( key == GLFW_KEY_3){
            glasses.reloadGlasses(glassesFile[2].c_str());
        } else {
            calibrated = !calibrated;
        }
    }
}
glm::mat4 externalToRotTrans(float* P_arr)
{
    Mat mat3x4 = P2PMat(P_arr);
    glm::mat4 mat4;
    mat3x4ToMat4(&mat3x4, mat4);
    return mat4;
}

//according to article http://stackoverflow.com/questions/22064084/how-to-create-perspective-projection-matrix-given-focal-points-and-camera-princ
glm::mat4 IntrinsicToProjection(Mat* intrinsicMat, int W, int H)
{
    float fx = intrinsicMat->at<float>(0, 0);
    float fy = intrinsicMat->at<float>(1, 1);
    float cx = intrinsicMat->at<float>(0, 2);
    float cy = intrinsicMat->at<float>(1, 2);
    float zmax = 1000.0f;
    float zmin = 0.1f;
    
    glm::mat4 projectionMat = { fx/cx,0,0,0,
        0, fy/cy,0,0,
        0,0,-(zmax+zmin)/(zmax-zmin),-1,
        0,0,2*zmax*zmin/(zmin-zmax),0};
    return projectionMat;
}

static void saveImage(Glasses& glasses) {
    uint8 *pImage_orig = (uint8*)malloc( srcWidth * srcHeight * 3 );
    glasses.readPixels(pImage_orig);
    uint8 *pImage_flipped_x = (uint8*)malloc( srcWidth * srcHeight * 3 );
    for( int i = 0; i < srcHeight; i++) {
        for( int j = 0; j < srcWidth; j++ ) {
            memcpy( pImage_flipped_x + (i * srcWidth + j) * 3, pImage_orig + ( (srcHeight-i)*srcWidth + j )*3, 3);
        }
    }
    // Fill in the compression parameter structure.
    jpge::params params;
    jpge::compress_image_to_jpeg_file(savedJpegFilePath.c_str(), srcWidth, srcHeight, 3, pImage_flipped_x, params);
    free( pImage_flipped_x );
    free( pImage_orig );
}

static void drawOpenGLGlasses(GLuint& dstTexture, Mat& frameOrig, Glasses& glasses, GLFWwindow* window, mat4& projectionMat, mat4& rotTransMat, bool shouldCopy ) {
    Mat* frame;
    Mat frameCopy;
    if( shouldCopy ) {
        frameCopy = frameOrig.clone();
        frame = &frameCopy;
    } else {
        frame = &frameOrig;
    }
    ///////////////////////////////////////////////////////////////////////////////////
    //glw window draw
    //NEXT convert from mat to opengl texture
    //http://stackoverflow.com/questions/16809833/opencv-image-loading-for-opengl-texture
    ///////////////////////////////////////////////////////////////////////////////////
    glBindTexture(GL_TEXTURE_2D, dstTexture);
    flip(*frame, *frame, 0); //flip x
    glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                 0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                 GL_RGBA,           // Internal colour format to convert to
                 (*frame).cols,        // Image width  i.e. 640 for Kinect in standard mode
                 (*frame).rows,        // Image height i.e. 480 for Kinect in standard mode
                 0,                 // Border width in pixels (can either be 1 or 0)
                 GL_BGR,            // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                 GL_UNSIGNED_BYTE,  // Image data type
                 (*frame).ptr()); // The actual image data itself
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //set new matrix
    glasses.setMatrices(projectionMat, rotTransMat);
    
    //draw glasses
    glasses.render(dstTexture, dstTexture, false);
    
    // Swap the buffers
    glfwSwapBuffers(window);
    
    // Check and call events
    glfwPollEvents();
}

int main()
{
    ////////////////
    //glw window
    ////////////////
    string glassesVsh;
    string glassesFsh;
    
    // Init GLFW
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    if( OPENGL_2_1 ) {
        glassesVsh = pathPrefix + "demo/LaputaDesktop3/VET/3dGlasses/3dGlassesVertexShaderGL2.1.vsh";
        glassesFsh = pathPrefix + "demo/LaputaDesktop3/VET/3dGlasses/3dGlassesFragmentShaderGL2.1.fsh";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    } else {
        glassesVsh = pathPrefix + "demo/LaputaDesktop3/VET/3dGlasses/3dGlassesVertexShaderGL.vsh";
        glassesFsh = pathPrefix + "demo/LaputaDesktop3/VET/3dGlasses/3dGlassesFragmentShaderGL.fsh";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    }
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 32); // 32x antialiasing, very aggressive
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    
    //GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", glfwGetPrimaryMonitor(), NULL);
    GLFWwindow* window = glfwCreateWindow(srcWidth, srcHeight, "OpenGL Mirror ", nullptr, nullptr); // Windowed
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);
    ////////////////
    
    //------------Initialization Begin---------------
    Mat cam_int = (Mat_<float>(3,3) << 650.66, 0, 319.50, 0, 650.94, 239.50, 0, 0, 1);
    glm::mat4 projectionMat4 = IntrinsicToProjection(&cam_int, srcWidth, srcHeight);
    
    //Scales
    float xScale = V_WIDTH/4-10;//V_WIDTH/4+10; //V_WIDTH/4-10;
    float yScale = V_HEIGHT/4+20;//V_HEIGHT/4+25; //V_HEIGHT/4+20;
    float zScale = (xScale+yScale)/2;
    
    //Shape Factors
    float shapeFactor[SHAPEUNITS] = //for Xz
    {0.0f,0.5f,0.4f, //headheight, eyebrows vertical pos, eyes vertical pos;
        0.1f,0.0f,0.3f, // eyes width, eyes height, eye seperation dis;
        0.0f,1.0f,0.5f, // cheeks z, nose-z extension, nose vertical pos;
        0.0f,0.0f,0.3f, // nose pointing up, mouth vertical pos, mouth width;
        0.0f,0.0f}; // eyes vertical diff, chin width;
    
    float P[NP] =      //space between glasses and candide3
    {
        0.0f, 0.0f, 0.0f,   //normalized rotation parameter
        0.0f, 0.0f, 1.0f,  // normalized translation parameter
    };
    
    vector<myvec3> verticesFromModel = readVertices(vertexFile);
    vector<triangle> faces = readFaces(faceFile);
    vector<myvec3> sfVertices = createSFVertices(verticesFromModel); //normalize, i.e., 40*40
    
    vector<myvec3> verticesGlass;
    vector<myvec3> verticesG_1st;
    
    if (GLASSON) {
        //Read glasses model
        bool res = loadOBJ((pathPrefix+"demo/LaputaDesktop3/VET/Glasses.obj").c_str(), verticesGlass);
        myrotate(verticesGlass, 90, 0, 1, 0);
        myrotate(verticesGlass, 180, 0, 0, 1);
        myscale(verticesGlass, 35, 35, 35);
        mytranslate(verticesGlass, 0, 10, -15);
        
        verticesG_1st = verticesGlass;
        mytransform(verticesG_1st, P);
    }
    
    vector<myvec3> verticesAdjusted(verticesFromModel);
    adjustShape(shapeFactor, verticesAdjusted, xScale, yScale, zScale); //based on the face, now it's xingze
    
    ////////////////
    //opengl objects
    ////////////////
    // add error checking here
    GLuint dstTexture;
    glGenTextures(1, &dstTexture);
    
    glasses.init(glassesVsh.c_str(),
                 glassesFsh.c_str(),
                 fragName,
                 glassesFile[0].c_str(),
                 faceFile.c_str(),
                 vertexFile.c_str(),
                 0, ASPECT_RATIO_4_3,
                 true, &verticesAdjusted ); //read adjusted coordinates directly from opengl.
    ////////////////
    
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
    for (int j = 0; j < V_HEIGHT; j++)
        for (int i = 0; i < V_WIDTH; i++){
            dMap[j][i] = 10000;
        }
    
    //dMap not useful
    
    //convert from normalized vertices and faces into map.
    createV2FMap(sfVertices, faces, map);
    
    //------------Initialization End---------------
    
    //------------   Tracking Begin ---------------
    if( SHOWTWOWIN ) {
        //Create tracking window
        namedWindow("Magic Mirror", 1);
        moveWindow("Magic Mirror", 400, 100);
    }
    vector<myvec3> vertices_1st(verticesAdjusted);
    mytransform(vertices_1st, P); // Initialized vertices array
    //mytransformWithAU(vertices_1st, P, AU);
    
    int iF = 0;
    int iM = 0;
    int trackFlag = 0;
    int maxIter = 400;
    float sigma = 0.001;
    float C = 100;
    float errDiff = 0.0000001;
    float errTh = 0;
    float errTh1 = 20; //10; //before first 40
    float errTh2 = 20; //0.6; //after first 40
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
    
    vector<myvec3> vertices_last(vertices_1st);
    Mat frame, frame_tmp, image; // "image" to be renamed as "frameG"
    Mat frameFromPic;
    frameFromPic = imread( pathPrefix + "demo/LinuxGL/sample/Tom.jpg", CV_LOAD_IMAGE_COLOR);
    
    while ( !glfwWindowShouldClose(window) ) 
    {
        frame = frameFromPic.clone();
        iF++;
                
        if (REALTIME) {
            if (trackFlag == 0){
                glm::mat4 rotTransMat4 = externalToRotTrans(P);
                drawOpenGLGlasses(dstTexture, frame, glasses, window, projectionMat4, rotTransMat4, true);
                
                if( SHOWTWOWIN ) {
                    frame_tmp = frame.clone();
                    //drawMeshOnImg_Per(frame_tmp, faces, vertices_1st);
                    
                    drawMeshOnImg_Per2(frame_tmp, faces, vertices_1st, cam_int);
                    //drawGlassOnImg_Per(frame_tmp, verticesG_1st);
                    imshow("Magic Mirror", frame_tmp);
                    //waitKey(0);
                }
                //if (waitKey(1) > 0)
                if ( calibrated )
                {
                    saveImage(glasses);
                    cvtColor(frame, image, CV_BGR2GRAY);
                    //tell the scaling factor of candide3, 3d mapping of vertices = vertices_1st and matrix. 3*4 matrix
                    //output sfImgRef.
                    genSFImg2(image, faces, vertices_1st, sfVertices, map, sfImgRef, cam_int);
                    for (int jY = 0; jY < SF_HEIGHT; jY++)
                        for (int jX = 0; jX < SF_WIDTH; jX++){
                            sfImgInit[jY][jX] = sfImgRef[jY][jX];
                        }
                    trackFlag = 1;
                }
                continue;
            }
            cvtColor(frame, image, CV_BGR2GRAY);
        }else{
            cvtColor(frame, image, CV_BGR2GRAY);
            if (iF == 1) {
                
                genSFImg2(image, faces, vertices_1st, sfVertices, map, sfImgRef, cam_int);
                for (int jY = 0; jY < SF_HEIGHT; jY++)
                    for (int jX = 0; jX < SF_WIDTH; jX++){
                        sfImgInit[jY][jX] = sfImgRef[jY][jX];
                    }
                continue;
            }
            
        }
        
        int curIter = 0;
        float rou = 1;
        
        vector<myvec3> vertices_cur(vertices_last);
        genSFImg2(image, faces, vertices_cur, sfVertices, map, sfImgCur, cam_int);
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
            vertices_last = vertices_1st;
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
                    vector<myvec3> vertices_tmp(verticesAdjusted);
                    mytransform(vertices_tmp, P_tmp);
                    //mytransformWithAU(vertices_tmp, P_tmp, AU);
                    genSFImg2(image, faces, vertices_tmp, sfVertices, map, sfImgTmp, cam_int);
                    for (int jY = 0; jY < SF_HEIGHT; jY++)
                        for (int jX = 0; jX < SF_WIDTH; jX++){
                            G[jY*SF_WIDTH+jX] += (1.0/K)*(sfImgTmp[jY][jX]-sfImgCur[jY][jX])/(k*sigma);
                        }
                    P_tmp[i] -= k*sigma;
                }
                
                Mat GM = Mat(SF_HEIGHT*SF_WIDTH, 1, CV_32FC(1), G);
                Mat GPM = Mat(SF_HEIGHT*SF_WIDTH, 1, CV_32FC(1), G);
                invert(GM, GPM, DECOMP_SVD);
                
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
            
            vector<myvec3> vertices_next(verticesAdjusted);
            mytransform(vertices_next, P_next);
            //mytransformWithAU(vertices_next, P_next, AU);
            genSFImg2(image, faces, vertices_next, sfVertices, map, sfImgNext, cam_int);
            for (int j = 0; j < SF_HEIGHT; j++)
                for (int i = 0; i < SF_WIDTH; i++){
                    sfImgDiff[j][i] = sfImgNext[j][i] -  sfImgRef[j][i];
                }
            err_next = errorCalc(sfImgDiff);
            
            //cout << "Err_cur: " << err_cur << endl;
            //cout << "Err_next: " << err_next << endl;
            
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
                    vector<myvec3> vertices_tmp1(verticesAdjusted);
                    mytransform(vertices_tmp1, P_next);
                    //mytransformWithAU(vertices_tmp1, P_next, AU);
                    genSFImg2(image, faces, vertices_tmp1, sfVertices, map, sfImgNext, cam_int);
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
        
        vector<myvec3> vertices_final(verticesAdjusted);
        mytransform(vertices_final, P_next);
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
                glm::mat4 rotTransMat4 = externalToRotTrans(P_next);
                drawOpenGLGlasses(dstTexture, frame, glasses, window, projectionMat4, rotTransMat4, SHOWTWOWIN );
                
                if( SHOWTWOWIN ) {
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
                }
                
            }else{
                glm::mat4 rotTransMat4 = externalToRotTrans(P_next);
                drawOpenGLGlasses(dstTexture, frame, glasses, window, projectionMat4, rotTransMat4, SHOWTWOWIN );
                
                if( SHOWTWOWIN ) {
                    drawMeshOnImg_Per2(frame, faces, vertices_final, cam_int);
                    //cout << "Waiting for the Next Frame ..." << endl;
                    imshow("Magic Mirror", frame);
                    //waitKey(0);
                }
            }
            
            
            vertices_last = vertices_final;
            for (int i = 0; i < NP; i++){
                P_cur[i] = P_next[i];
                P_tmp[i] = P_next[i];
            }
            
            
        }else{
            
            glm::mat4 rotTransMat4 = externalToRotTrans(P);
            drawOpenGLGlasses(dstTexture, frame, glasses, window, projectionMat4, rotTransMat4, SHOWTWOWIN );
            
            //
            
            for (int jY = 0; jY < SF_HEIGHT; jY++)
                for (int jX = 0; jX < SF_WIDTH; jX++){
                    sfImgRef[jY][jX] = sfImgInit[jY][jX];
                }
            
            if( SHOWTWOWIN ) {
                //reset the size.
                //drawMeshOnImg_Per(frame, faces, vertices_1st);
                drawGlassOnImg_Per(frame, verticesG_1st);
                //waitKey(0);
                //cout << "Debuging ..." << endl;
                imshow("Magic Mirror", frame);
                //waitKey(0);
            }
            
            vertices_last = vertices_1st;
            
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
        
        if(waitKey(1) >= 0) break;
        
    }
    
    //------------   Tracking End  ---------------
    
    return 0;
}
