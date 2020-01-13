#ifndef ITYPES_H
#define ITYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>

#ifndef MAX_NAME
#define MAX_NAME         (256)
#endif

#ifndef MAX_PATH
#define MAX_PATH        (1024)
#endif

#ifndef MAX_LINE
#define MAX_LINE        (1024*2)
#endif

/* 获取定义的数组长度 */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef max
#define max(x,y)  ( x>y?x:y )
#endif

#ifndef min
#define min(x,y)  ( x<y?x:y )
#endif


typedef uint16_t ibe16;
typedef uint32_t ibe32;
typedef uint64_t ibe64;

typedef uint16_t ile16;
typedef uint32_t ile32;
typedef uint64_t ile64;

#endif