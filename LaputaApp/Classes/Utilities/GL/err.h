//
//  err.h
//  Laputa
//
//  Created by Howard Wang on 15-5-14.
//
//

#ifndef Laputa_err_h
#define Laputa_err_h

#include <stdio.h>

inline void getGLErr(const char* prefix){
    int err = glGetError();
    if( err != 0 ) {
        printf("%s err=%d\r\n", prefix, err);
    }
}
#endif
