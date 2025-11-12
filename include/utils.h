//
// Created by zheng on 2025/11/12.
//

#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

#define CHANCE_OF_IO_REQUEST 10
#define CHANCE_OF_IO_COMPLETE 4

void os_srand(unsigned int seed);
int os_rand(void);

int IO_request(void);
int IO_complete(void);

#endif //UTILS_H
