#include "LandMark.h"

int main(int argc, const char * argv[])
{
    LandMark* lm = new LandMark();
    string faceImg = "/data/laputa/images/default/testImg01.png";
    string jsonObject;
    if(!lm->ProcessImage( faceImg, jsonObject )) {
        cout<<jsonObject;
    }
    delete(lm);
}
