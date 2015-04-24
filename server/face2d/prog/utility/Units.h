#ifndef __UNITS_H__
#define __UNITS_H__

//assume it's linux system
#include <stdint.h>
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define MAX(a, b) (a > b)? a : b
#define MIN(a, b) (a < b)? a : b
#define CLIP( val, max, min ) ( val >  max ) ? max: ( ( val < min ) ? min: val)

#define MAX_U32 0xffffffff
#define MAX_S32 0x0fffffff

#endif
