#include "include/clock.h"

static int clock_time = 0;

void init_clock(void) {
    clock_time = 0;
}

int current_clock(void) {
    return clock_time;
}

void next_tick(void) {
    clock_time++;
}