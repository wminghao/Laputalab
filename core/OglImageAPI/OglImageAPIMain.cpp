#include "OglImageAPI.h"

//math library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

int main(int argc, const char * argv[])
{
    const char* img = "./Tom.jpg";
    OglImageAPI* lm = new OglImageAPI();
    string faceImg = (argc > 1 )?argv[1]:img;
    
    glm::mat4 model(1.0);
    string errReason;
    if(!lm->ProcessImage( faceImg, 
                          model,
                          errReason )) {
        cout << "success"<<endl;
        for (int j=0; j<4; j++){
            for (int i=0; i<4; i++){
                printf("%f ",model[i][j]);
            }
            printf("\n");
        }
    } else { 
       cout << "err"<<errReason;
    }
    delete(lm);
}
