#ifndef __FWK_TIME_H__
#define __FWK_TIME_H__

#include "Units.h"
#include <sys/time.h>
#include <stdio.h>

inline u32 count_bits(u32 n) {
    unsigned int c; // c accumulates the total bits set in v
    for (c = 0; n; c++) {
        n &= n - 1; // clear the least significant bit set
    }
    return c;
}

inline u64 getEpocTime() {
    struct timeval tv;

    gettimeofday(&tv, NULL);    
    unsigned long long millisecondsSinceEpoch =
        (unsigned long long)(tv.tv_sec) * 1000 +
        (unsigned long long)(tv.tv_usec) / 1000;
    
    //LOG( "%llu\n", millisecondsSinceEpoch);
    return (u64)millisecondsSinceEpoch;
}

#endif
