//
//  platform.h
//  LaputaMobile App
//
//  Created by Howard Wang on 15-5-12.
//  Copyright (c) 2015年 Howard Wang. All rights reserved.
//

#ifndef __platform_h__
#define __platform_h__

//#define DESKTOP_MAC

#ifdef DESKTOP_MAC
#include <OpenGL/gl.h>
#else //DESKTOP_MAC
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif //DESKTOP_MAC

#endif
