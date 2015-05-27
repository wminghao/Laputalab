//
//  uniit.h
//  Laputa
//
//  Created by Howard Wang on 15-5-27.
//
//

#ifndef Laputa_uniit_h
#define Laputa_uniit_h

struct Triangle
{
public:
    Triangle() {
        a = 0;
        b = 0;
        c = 0;
    }

    Triangle(int aa,
             int bb,
             int cc){
        a = aa;
        b = bb;
        c = cc;
    }
    int a;
    int b;
    int c;
};

struct Vector4f
{
    Vector4f() {
        x = 0;
        y = 0;
        z = 0;
        w = 0;
    }
    Vector4f(float xx,
             float yy,
             float zz,
             float ww){
        x = xx;
        y = yy;
        z = zz;
        w = ww;
    }
    float x;
    float y;
    float z;
    float w;
};

struct Vector3f
{
    Vector3f() {
        x = 0;
        y = 0;
        z = 0;
    }
    Vector3f(float xx,
             float yy,
             float zz){
        x = xx;
        y = yy;
        z = zz;
    }
    float x;
    float y;
    float z;
    
};
struct Vector2f
{
    Vector2f() {
        x = 0;
        y = 0;
    }
    Vector2f(float xx,
             float yy){
        x = xx;
        y = yy;
    }
    float x;
    float y;
};

#endif
