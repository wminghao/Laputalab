#ifndef __OUTPUT__
#define __OUTPUT__
#include "Logger.h"

#define ENABLE_LOGGER
#ifdef ENABLE_LOGGER
//#define OUTPUT(...) printf(__VA_ARGS__)
#define OUTPUT Logger::log
#define ASSERT(x) assert(x)
#else 
#define OUTPUT(...) 
#define ASSERT(x) 
#endif

#endif
