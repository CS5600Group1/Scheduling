//
// Created by zheng on 2025/11/11.
//

#ifndef CLOCK_H
#define CLOCK_H

static int clock_time = 0;

void init_clock() {
    clock_time = 0;
}
int current_clock() {
    return clock_time;
}
void next_tick() {
    clock_time++;
}

#endif //CLOCK_H
