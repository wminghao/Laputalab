#ifndef __LOG_H__
#define __LOG_H__

#include <fstream>
#include <assert.h>
#include <stdarg.h>

#define LOG Logger::log

class Logger
{
 public:
    static void initLog( const char* programName, bool bNoShow = false);
    static void log( const char * fmt, ... );

 private:
    Logger();
};

#endif //__LOG_H__
