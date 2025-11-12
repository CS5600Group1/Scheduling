//
// Created by zheng on 2025/11/12.
//

#include <stdlib.h>

#include "include/utils.h"

void os_srand(unsigned int seed) {
    srand(seed);
}

int os_rand(void) {
    return rand();
}

int IO_request(void) {
    return (os_rand() % CHANCE_OF_IO_REQUEST) == 0;
}

int IO_complete(void) {
    return (os_rand() % CHANCE_OF_IO_COMPLETE) == 0;
}

