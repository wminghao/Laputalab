#ifndef __OUTPUT__
#define __OUTPUT__
#include "Logger.h"

#define DEBUG
#ifdef DEBUG
//#define OUTPUT(...) printf(__VA_ARGS__)
#define OUTPUT Logger::log
#define ASSERT(x) assert(x)
#else 
#define OUTPUT(...) 
#define ASSERT(x) 
#endif

#endif
