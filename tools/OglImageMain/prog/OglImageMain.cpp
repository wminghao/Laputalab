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

//API
#include "OglImageAPI.h"

using namespace std;
using namespace cv;
using namespace Utilities_Namespace;

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
    exit(-1);
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

glm::mat4 IntrinsicToOrthoProjection(int W, int H)
{
    
    float aspect = (float)W/(float)H;
    return glm::ortho(-aspect, aspect, -1.0f, 1.0f, (float)-DIST, (float)DIST);
    //return glm::ortho((float)-W/2, (float)W/2, (float)-H/2, (float)H/2, 0.0f, (float)DIST//float ratio = 1;
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
                src = (uint8*)buffer + ( (srcHeight-1-i)*srcWidth + j )*4;
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
        sprintf(buf, "OOM: srcWidth = %d, srcHeight=%d", srcWidth, srcHeight);
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
    //glasses.setMatrices(projectionMat, rotTransMat);
    //TEST ONLY
    glm::mat4 bbb(1.0);
    glasses.setMatrices(projectionMat, bbb);
      
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
    Size srcSize = frame_orig.size();
    
    if( srcSize.width * srcSize.height > MAX_WIDTH_X_HEIGHT ) {
        char err[300];
        sprintf(err, "OOM: Input size too big. width=%d, height=%d", srcSize.width, srcSize.height);
        errReason = err;
        return -1;
    } else if( !srcSize.width || !srcSize.height ) {
        char err[300];
        sprintf(err, "Error: Input size wrong. width=%d, height=%d", srcSize.width, srcSize.height);
        errReason = err;
        return -1;
    }

    resize(frame_orig, frame, Size(), AA_FACTOR, AA_FACTOR, INTER_CUBIC);
    frame_orig.release();

    ASPECT_RATIO aspectRatio = ASPECT_RATIO_4_3;
    float zRotateInDegree = 0;
    if( srcSize.width * 3 == srcSize.height * 4 ) {
        aspectRatio = ASPECT_RATIO_4_3;
        OUTPUT("image asepect ratio: 4/3\r\n");
    } else if(srcSize.width * 9 == srcSize.height * 16 ) {
        aspectRatio = ASPECT_RATIO_16_9;
        OUTPUT("image asepect ratio: 16/9\r\n");
    } else if(srcSize.width == srcSize.height) {
        aspectRatio = ASPECT_RATIO_1_1;
        OUTPUT("image asepect ratio: 1/1\r\n");
    } else if( srcSize.width * 4 == srcSize.height * 3 ) {
        aspectRatio = ASPECT_RATIO_4_3;
        zRotateInDegree = 90;
        OUTPUT("image asepect ratio: 3/4\r\n");
    } else if( srcSize.width * 16 == srcSize.height * 9 ) {
        aspectRatio = ASPECT_RATIO_16_9;
        zRotateInDegree = 90;
        OUTPUT("image asepect ratio: 9/16\r\n");
    } else {
        //errReason = "image asepect ratio: unknown!";
        //return -1;
        OUTPUT("image asepect ratio: unknown!");
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
        errReason = "OSMesaCreateContextExt failed!";
        return -1;
    }
    void* buffer = malloc(srcWidth*srcHeight*4*sizeof(GLubyte));
    /* Bind the buffer to the context and make it current */
    if (!OSMesaMakeCurrent( ctx, buffer, GL_UNSIGNED_BYTE, srcWidth, srcHeight )) {
        errReason="OSMesaMakeCurrent failed!";
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
    glm::mat4 projectionMat4 = IntrinsicToOrthoProjection(srcWidth/AA_FACTOR, srcHeight/AA_FACTOR);
            
    ////////////////
    OglImageAPI lm;
    mat4 rotTransMat4;
    if( !lm.ProcessImage( inputFile,
                          rotTransMat4,
                          errReason )) {
        cout << "success"<<endl;
        for (int j=0; j<4; j++){
            for (int i=0; i<4; i++){
                printf("%f ", rotTransMat4[i][j]);
            }
            printf("\n");
        }
    } else {
        return -1;
    }

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
                 zRotateInDegree, aspectRatio,
                 false, NULL,
                 true,
                 0.10); //Ran's glasses are all shifted up by 5%
    drawOpenGLGlasses(dstTexture, frame, glasses, projectionMat4, rotTransMat4);
    frame.release();
    if( !saveBuffer(buffer, outputFile, srcWidth, srcHeight, errReason) )  {
        return -1;
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
