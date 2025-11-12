//
// Created by zheng on 2025/11/12.
//

#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

#define CHANCE_OF_IO_REQUEST 10
#define CHANCE_OF_IO_COMPLETE 4

void os_srand(int seed){
    srand(seed);
}

void os_rand() {
    return rand();
}

int IO_request(){
    if ( os_rand() % CHANCE_OF_IO_REQUEST == 0 )
        return 1;
    else
        return 0;
}

int IO_complete(){
    if ( os_rand() % CHANCE_OF_IO_REQUEST == 0 )
        return 1;
    else
        return 0;
}

#endif //UTILS_H
