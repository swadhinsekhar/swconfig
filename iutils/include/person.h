#ifndef PERSON_H
#define PERSON_H

#include <stdint.h>



typedef uint64_t BSP_UINT64;

typedef struct {
    char city[32];
    char * desc;
    BSP_UINT64 code;
}address;

typedef struct  {
    uint8_t age;
    char name[64];
    uint32_t ids[10];
    char * info;
    address addr;
    address addrs2[5];
    address * addr_p;
    uint32_t addr_arr_n;
    address * addr_arr;
}person;





#endif