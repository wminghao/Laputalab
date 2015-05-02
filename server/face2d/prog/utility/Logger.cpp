#include <string>
#include <sstream>
#include <syslog.h>
#include <iomanip>
#include <fstream>
#include "Logger.h"

bool g_bNoShow = false;

void Logger::initLog( const char* syslogName, bool bNoShow)
{
    g_bNoShow = bNoShow;
    if( !bNoShow ) {
        openlog( syslogName, LOG_PID, LOG_USER );
    }
}

void Logger::log( const char * fmt, ... )
{
    if( !g_bNoShow ) {
        va_list args;
        
        /* initialize valist for num number of arguments */
        va_start(args, fmt);
        vsyslog( LOG_DEBUG, fmt, args );
        va_end(args);
    }
}
