//
//  platform.h
//  LaputaDesktop
//
//  Created by Howard Wang on 15-5-12.
//  Copyright (c) 2015年 Howard Wang. All rights reserved.
//

#ifndef __platform_h__
#define __platform_h__

#ifdef __MACH__
#include <OpenGL/gl.h>
#include <OpenGL/gl3.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/CGLTypes.h>
#else //__MACH__
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif //__MACH__

#endif
