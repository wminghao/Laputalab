//
//  main.cpp
//  3DGlasses
//
//  Created by Xingze on 4/21/14.
//  Copyright (c) 2014 Xingze. All rights reserved.
//

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <tr1/cstdint>
 
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//Facial Model File
const char * pFile = "/Users/Xavier/CodingProject/3DGlassesRender/3DGlasses/Glasses.obj";
//const char * pFile = "/Users/Xavier/CodingProject/3DGlassesRender/3DGlasses/RayBanz.obj";


using namespace glm;
using namespace std;
using namespace cv;

#include "shader.hpp"
#include "objloader.hpp"

int main( void )
{

    //Load model with ASSIMP
    Assimp::Importer importer;
    /*
    const aiScene* scene = importer.ReadFile( pFile,
                                             aiProcess_CalcTangentSpace       |
                                             aiProcess_Triangulate            |
                                             aiProcess_JoinIdenticalVertices  |
                                             aiProcess_SortByPType);
    */
    const aiScene* scene = importer.ReadFile( pFile,
                                             aiProcess_Triangulate);
    
    if (!scene){
        cout << "Model loading error" << endl;
    }
    
    aiMesh * meshF = scene->mMeshes[0]; //Frame mesh
    aiMesh * meshL = scene->mMeshes[1]; //Lens mesh
    //cout << scene->mNumMeshes << endl;
    vector<vec3> verticesF;
    vector<vec3> verticesL;
    
    for (int i = 0; i < meshF->mNumFaces; i++){
        const aiFace & face = meshF->mFaces[i];
        
        for (int j = 0; j < 3; j++){
            aiVector3D * pos = &(meshF->mVertices[face.mIndices[j]]);
            vec3 v(pos->x, pos->y, pos->z);
            verticesF.push_back(v);
        }
    }
    
    for (int i = 0; i < meshL->mNumFaces; i++){
        const aiFace & face = meshL->mFaces[i];
        
        for (int j = 0; j < 3; j++){
            aiVector3D * pos = &(meshL->mVertices[face.mIndices[j]]);
            vec3 v(pos->x, pos->y, pos->z);
            verticesL.push_back(v);
        }
    }
    
    
    
    
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 640, 480, "3D Glasses", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
    
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    
    
    
    
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.3f, 0.0f);
    
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    // Create and compile our GLSL program from the shaders
    GLuint programIDL = LoadShaders( "/Users/Xavier/CodingProject/3DGlassesRender/3DGlasses/SimpleVertexShaderL.vertexshader", "/Users/Xavier/CodingProject/3DGlassesRender/3DGlasses/SimpleFragmentShaderL.fragmentshader" );
    
    GLuint programIDF = LoadShaders( "/Users/Xavier/CodingProject/3DGlassesRender/3DGlasses/SimpleVertexShaderF.vertexshader", "/Users/Xavier/CodingProject/3DGlassesRender/3DGlasses/SimpleFragmentShaderF.fragmentshader" );
    
    // Get a handle for our "MVP" uniform
	GLuint MatrixIDL = glGetUniformLocation(programIDL, "MVP");
    
    // Get a handle for our "MVP" uniform
	GLuint MatrixIDF = glGetUniformLocation(programIDF, "MVP");
    
    //vector<vec3> vertices;
    //bool res = loadOBJ("/Users/Xavier/CodingProject/3DGlassesRender/3DGlasses/Glasses.obj", vertices);
    
    GLuint vertexbufferF;
    glGenBuffers(1, &vertexbufferF);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbufferF);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, verticesF.size() * sizeof(glm::vec3), &verticesF[0], GL_STATIC_DRAW);
    
    GLuint vertexbufferL;
    glGenBuffers(1, &vertexbufferL);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbufferL);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, verticesL.size() * sizeof(glm::vec3), &verticesL[0], GL_STATIC_DRAW);
    
    
            
    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    // Or, for an ortho camera :
    //glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates
            
    // Camera matrix
    glm::mat4 View       = glm::lookAt(
                                    glm::vec3(5,6,5), // Camera is at (4,3,3), in World Space
                                    glm::vec3(0,0,0), // and looks at the origin
                                    glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                                        );
            // Model matrix : an identity matrix (model will be at the origin)
            //glm::mat4 Model      = glm::mat4(1.0f);
    mat4 Model_translation = translate(mat4(1.0f), vec3(0,0,0));
    mat4 Model_rotate = rotate(mat4(1.0f), 90.0f, vec3(0,1,0));
    mat4 Model_scale = scale(mat4(1.0f), vec3(0.6,0.6,0.6));
    mat4 Model = Model_translation * Model_rotate * Model_scale;
            
            // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around
    
    
    //Adding texture
    //glEnable(GL_TEXTURE_2D);
    
    // Create Texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID); // 2d texture (x and y size)
    
    cv::Mat texture_cv = cv::imread("/Users/Xavier/CodingProject/3DGlassesRender/3DGlasses/test.bmp");
    
    // 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image,
    // border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.
    glTexImage2D(GL_TEXTURE_2D, 0, 3, texture_cv.cols, texture_cv.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, texture_cv.data);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    
    // These are necessary if using glTexImage2D instead of gluBuild2DMipmaps
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    
    do{
        // Clear the screen
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        
        
        // Use our shader
        glUseProgram(programIDL);
            
        glUniformMatrix4fv(MatrixIDL, 1, GL_FALSE, &MVP[0][0]);
            
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbufferL);
        glVertexAttribPointer(
                                  0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                                  3,                  // size
                                  GL_FLOAT,           // type
                                  GL_FALSE,           // normalized?
                                  0,                  // stride
                                  (void*)0            // array buffer offset
                                  );
            
            
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, verticesL.size()); // 3 indices starting at 0 -> 1 triangle
        glDisableVertexAttribArray(0);
        
        // Use our shader
        glUseProgram(programIDF);
        
        glUniformMatrix4fv(MatrixIDF, 1, GL_FALSE, &MVP[0][0]);
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbufferF);
        glVertexAttribPointer(
                              0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        
        
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, verticesF.size()); // 3 indices starting at 0 -> 1 triangle
        glDisableVertexAttribArray(0);
        
        
        
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
          glfwWindowShouldClose(window) == 0 );

    
    // OpenGL Ending ...
            
    
    // Cleanup VBO
    glDeleteBuffers(1, &vertexbufferF);
    glDeleteBuffers(1, &vertexbufferL);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programIDF);
    glDeleteProgram(programIDL);
    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    
    
    return 0;
    
    
    //Tracking Code Ending...
    
 }

