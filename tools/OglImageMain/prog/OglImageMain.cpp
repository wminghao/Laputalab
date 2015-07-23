#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>
#include <sys/signal.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   /* For open(), creat() */
#include <string>
#include <assert.h>
#include "utility/Output.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "OglImageMain.h"
#include "utilities.h"

// GLFW
#include "GL/osmesa.h"

//glasses
#include "glasses.h"

using namespace std;
using namespace cv;

//////////////////////////////////////////////
//I/O code
//////////////////////////////////////////////

void fnExit (void)
{
    LOG("----OglImageMain Exited!");
}
void handlesig( int signum )
{
    LOG( "Exiting on signal: %d", signum  );
    LOG( "OglImageMain just crashed, see stack dump below." );
    LOG( "---------------------------------------------");
    void *array[10];
    size_t bt_size;

    // get void*'s for all entries on the stack
    bt_size = backtrace(array, 10);

    // print out all the frames to stderr
    char **bt_syms = backtrace_symbols(array, bt_size);
    for (size_t i = 1; i < bt_size; i++) {
        //size_t len = strlen(bt_syms[i]);
        LOG("%s", bt_syms[i]);
    }
    free( bt_syms );
    LOG( "---------------------------------------------");
}


bool doRead( int fd, char *buf, size_t len ) {
    size_t bytesRead = 0;

    while ( bytesRead < len ) {
        size_t bytesToRead = len - bytesRead;
        int t = read( fd, buf + bytesRead, bytesToRead );
        if ( t <= 0 ) {
            OUTPUT("read() failed: %s\r\n", strerror(errno) );

            return false;
        }
        bytesRead += t;
    }

    return true;
}

bool doWrite( int fd, const char *buf, size_t len ) {
    size_t bytesWrote = 0;

    while ( bytesWrote < len ) {
        size_t bytesToWrite = len - bytesWrote;
        int t = write( fd, buf + bytesWrote, bytesToWrite );
        if ( t <= 0 ) {
            OUTPUT("write() failed: %s \r\n", strerror(errno) );
            return false;
        }

        bytesWrote += t;
    }

    return true;
}

//////////////////////////////////////////////
//OpenGL + OpenCV code
//////////////////////////////////////////////
#define REALTIME 1
#define GLASSON 0
#define OPENGL_2_1 1

const int MAX_WIDTH_X_HEIGHT = 960*960;//max memory on ytiyan server, 1Gbytes, too big.

const int AA_FACTOR = 4; //4 uses too much memory here.

//Facial Model Source File
const string vertexFile = pathPrefix + "demo/LaputaDesktop3/VET/facemodel/vertexlist_113.wfm";
const string faceFile = pathPrefix + "demo/LaputaDesktop3/VET/facemodel/facelist_184.wfm";
const string glassesFilePrefix = "/shared/3dmodels/";
const char* fragName = "outFrag";

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

static bool saveBuffer(void* buffer, string& fileToSave, int srcWidth, int srcHeight, string& errReason) {
    //first flip the x axis
    uint8 *pImage_flipped_x = (uint8*)malloc( srcWidth * srcHeight * 4 );
    if( pImage_flipped_x ) {
        //convert from RGBA to BGRA
        uint8* src;
        uint8* dst;    

        for( int i = 0; i < srcHeight; i++) {
            for( int j = 0; j < srcWidth; j++ ) {
                src = (uint8*)buffer + ( (srcHeight-i)*srcWidth + j )*4;
                dst = pImage_flipped_x + (i * srcWidth + j) * 4;
                *dst = *(src+2);
                *(dst+1) = *(src+1);
                *(dst+2) = *src;
                *(dst+3) = *(src+3);
            }
        }
        //then resize the image
        Mat origImage( srcHeight, srcWidth, CV_8UC4, pImage_flipped_x);
        Mat finalImage;
        float ratio = (float)1/(float)AA_FACTOR;
        resize(origImage, finalImage, Size(), ratio, ratio, INTER_AREA);
        //finally save the image
        imwrite(fileToSave.c_str(), finalImage);
        free( pImage_flipped_x );

        return true;
    } else {
        char buf[200];
        sprintf(buf, "OOM: srcWidth = %d, srcHeight=%d\r\n", srcWidth, srcHeight);
        errReason = buf;
        return false;
    }
}

static void drawOpenGLGlasses(GLuint& dstTexture, Mat& frameOrig, Glasses& glasses, mat4& projectionMat, mat4& rotTransMat ) {
    Mat* frame = &frameOrig;

    ///////////////////////////////////////////////////////////////////////////////////
    //NEXT convert from mat to opengl texture
    //http://stackoverflow.com/questions/16809833/opencv-image-loading-for-opengl-texture
    ///////////////////////////////////////////////////////////////////////////////////
    glBindTexture(GL_TEXTURE_2D, dstTexture);
    flip(*frame, *frame, 0); //flip x
    
    //OUTPUT("rows=%d, cols=%d, ptr=0x%x", (*frame).rows, (*frame).cols, (*frame).ptr());
    if( (*frame).ptr() && (*frame).cols > 0 && (*frame).rows > 0 ) { 
        glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                     0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                     GL_RGBA,           // Internal colour format to convert to
                     (*frame).cols,        // Image width  i.e. 640 for Kinect in standard mode
                     (*frame).rows,        // Image height i.e. 480 for Kinect in standard mode
                     0,                 // Border width in pixels (can either be 1 or 0)
                     GL_BGR,            // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                     GL_UNSIGNED_BYTE,  // Image data type
                     (*frame).ptr());   // The actual image data itself
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    //set new matrix
    glasses.setMatrices(projectionMat, rotTransMat);

    //draw glasses
    glasses.render(dstTexture, dstTexture, false);
}

int ProcessFile( string& iFilePath, string& oFilePath, string& gName, string & errReason ) {
    string inputFile = iFilePath;
    string outputFile = oFilePath;
    string glassesFile = glassesFilePrefix+gName+"/"+gName+".obj";

    OUTPUT("input file=%s", inputFile.c_str());
    OUTPUT("output file=%s", outputFile.c_str());
    OUTPUT("glasses file=%s", glassesFile.c_str());
    
    Mat frame, frame_orig;
    frame_orig = imread( inputFile, CV_LOAD_IMAGE_COLOR);
    
    OUTPUT("input size w=%d, h=%d\r\n", frame_orig.size().width, frame_orig.size().height);
    
    //double the size
    resize(frame_orig, frame, Size(), AA_FACTOR, AA_FACTOR, INTER_CUBIC);
    Size srcSize = frame_orig.size();
    frame_orig.release();

    if( srcSize.width * srcSize.height > MAX_WIDTH_X_HEIGHT ) {
        char err[300];
        sprintf(err, "OOM: Input size too big. width=%d, height=%d\r\n", srcSize.width, srcSize.height);
        errReason = err;
        return -1;
    }

    ASPECT_RATIO aspectRatio = ASPECT_RATIO_4_3;
    if( srcSize.width * 3 == srcSize.height * 4 ) {
        aspectRatio = ASPECT_RATIO_4_3;
        OUTPUT("image asepect ratio: 4/3\r\n");
    } else if(srcSize.width * 9 == srcSize.height * 16 ) {
        aspectRatio = ASPECT_RATIO_16_9;
        OUTPUT("image asepect ratio: 16/9\r\n");
    } else if(srcSize.width == srcSize.height) {
        aspectRatio = ASPECT_RATIO_1_1;
        OUTPUT("image asepect ratio: 1/1\r\n");
    } else {
        errReason = "image asepect ratio: unknown\r\n";
        return -1;
    }
    
    //manually resize to achive aa
    const int srcWidth = srcSize.width * AA_FACTOR;
    const int srcHeight = srcSize.height * AA_FACTOR; 
    Glasses glasses(srcWidth, srcHeight, false);
    
    ////////////////
    //osmesa context
    ////////////////
    const GLint zdepth = 32; //32 bits z depth buffer
    const GLint stencil = 8; //8 bits stencil
    const GLint accum = 0; //accumulation buffer
    OSMesaContext ctx = OSMesaCreateContextExt( OSMESA_RGBA, zdepth, stencil, accum, NULL);
    if(!ctx) {
        errReason = "OSMesaCreateContextExt failed!\n";
        return -1;
    }
    void* buffer = malloc(srcWidth*srcHeight*4*sizeof(GLubyte));
    /* Bind the buffer to the context and make it current */
    if (!OSMesaMakeCurrent( ctx, buffer, GL_UNSIGNED_BYTE, srcWidth, srcHeight )) {
        errReason="OSMesaMakeCurrent failed!\n";
        return -1;
    }
    
    string glassesVsh;
    string glassesFsh;    
    if( OPENGL_2_1 ) {
        glassesVsh = pathPrefix + "demo/LaputaDesktop3/VET/3dGlasses/3dGlassesVertexShaderGL2.1.vsh";
        glassesFsh = pathPrefix + "demo/LaputaDesktop3/VET/3dGlasses/3dGlassesFragmentShaderGL2.1.fsh";
    } else {
        glassesVsh = pathPrefix + "demo/LaputaDesktop3/VET/3dGlasses/3dGlassesVertexShaderGL.vsh";
        glassesFsh = pathPrefix + "demo/LaputaDesktop3/VET/3dGlasses/3dGlassesFragmentShaderGL.fsh";
    }
    ////////////////
    
    //------------Initialization Begin---------------
    int curWidth;
    switch( aspectRatio ) {
    case ASPECT_RATIO_16_9: {
        curWidth = 360/2-0.5;
        break;
    }
    case ASPECT_RATIO_4_3: {
        curWidth = 480/2-0.5;
        break;
    }
    case ASPECT_RATIO_1_1: 
    default: {
        curWidth = 640/2-0.5;
        break;
    }
    }
    Mat cam_int = (Mat_<float>(3,3) << 650.66, 0, 319.50, 0, 650.94, curWidth, 0, 0, 1);
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
                 glassesFile.c_str(),
                 faceFile.c_str(),
                 vertexFile.c_str(),
                 0, aspectRatio,
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
    Mat frame_tmp, image; // "image" to be renamed as "frameG"

    //only process once.
    for(int i=0; i<1; i++)
    {
        frame = frame.clone();
        iF++;
                
        if (REALTIME) {
            if (trackFlag == 0){
                glm::mat4 rotTransMat4 = externalToRotTrans(P);
                drawOpenGLGlasses(dstTexture, frame, glasses, projectionMat4, rotTransMat4);
                frame.release();
		if( !saveBuffer(buffer, outputFile, srcWidth, srcHeight, errReason) )  {
                    return -1;
                }
                if ( 0 ) //disable the calibration
                {
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
                drawOpenGLGlasses(dstTexture, frame, glasses, projectionMat4, rotTransMat4 );
            }else{
                glm::mat4 rotTransMat4 = externalToRotTrans(P_next);
                drawOpenGLGlasses(dstTexture, frame, glasses, projectionMat4, rotTransMat4 );
            }
            
            
            vertices_last = vertices_final;
            for (int i = 0; i < NP; i++){
                P_cur[i] = P_next[i];
                P_tmp[i] = P_next[i];
            }
            
            
        }else{
            
            glm::mat4 rotTransMat4 = externalToRotTrans(P);
            drawOpenGLGlasses(dstTexture, frame, glasses, projectionMat4, rotTransMat4);
            
            //
            
            for (int jY = 0; jY < SF_HEIGHT; jY++)
                for (int jX = 0; jX < SF_WIDTH; jX++){
                    sfImgRef[jY][jX] = sfImgInit[jY][jX];
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
    }
    
    //------------   Tracking End  ---------------
    free( buffer );
    OSMesaDestroyContext( ctx );
    return 0;
}

///////////////////////////////
//main function
///////////////////////////////
int main()
{
    //ignore sig pipe, any io to an invalid pipe will return properly
    signal(SIGPIPE, SIG_IGN);
    signal( SIGHUP, SIG_IGN ); //ignore hangup
    signal( SIGSEGV, handlesig );
    signal( SIGFPE, handlesig );
    signal( SIGBUS, handlesig );
    signal( SIGSYS, handlesig );
    signal( SIGTERM, handlesig );
    signal( SIGQUIT, handlesig );
    signal( SIGINT, handlesig );
    signal( SIGILL, handlesig );
    signal( SIGABRT, handlesig );
    signal( SIGALRM, handlesig );
    signal( SIGXCPU, handlesig ); //CPU limit
    // SIGKILL command cannot be caught
    atexit(fnExit);

    Logger::initLog("OglImageMainProc");

    OUTPUT("------OglImageMainProc started!\r\n");
    bool bWorking = true;

    while ( true ) {
        char lenBuf[4];
        bWorking = doRead( 0, lenBuf, 4 );
        
        if( bWorking ) {
            int pathLen = 0;
            memcpy(&pathLen, lenBuf, 4);
            OUTPUT("------OglImageMain read data, size=%d\n", pathLen);
            
            if( pathLen > 0 ) { 
                bool bIsSuccess = false;
                char buf[pathLen];
                bWorking = doRead( 0, buf, pathLen );
                if( bWorking ) {
                    buf[pathLen]='\0';
                    char iFilePathStr[200];
                    char oFilePathStr[200];
                    char gNameStr[100];
                    sscanf(buf, "input=%[^&]&output=%[^&]&glasses=%s", iFilePathStr, oFilePathStr, gNameStr);
                    string iFilePath = iFilePathStr;
                    string oFilePath = oFilePathStr;
                    string gName = gNameStr;
                    if( iFilePath.length() && oFilePath.length() && gName.length() ) {
                        string errReason;
                        string result;
                        
                        int ret = ProcessFile( iFilePath, oFilePath, gName, errReason );
                        if( !ret ) {
                            result = "{ \"status\":\"success\",\"message\":\"ok\"}";
                        } else {
                            result = "{ \"status\":\"error\",\"message\":\"" + errReason + "\"}";
                            OUTPUT("OglImageMain Processing failed. input=%s, glasses=%s, errReason=%s", iFilePathStr, gNameStr, errReason.c_str());
                        }
                        
                        unsigned int bufLen = result.length();
                        doWrite( 1, (char*)&bufLen, 4);
                        doWrite( 1, result.c_str(), bufLen);                
                        OUTPUT("Len:%d, buf:%s", bufLen, result.c_str());
                    } else {
                        OUTPUT("Parsing error");
                    }
                }
            } else {
                break;
            }
        }
    }
}
