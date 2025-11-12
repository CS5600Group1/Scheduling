//
// Created by zheng on 2025/11/11.
//

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "job.h"

typedef struct {
    int total_simulate_time;
    int total_number_of_job;
    int shortest_job_completion_time;
    int Longest_job_completion_time;
    int average_completion;
    int average_ready;
} Global_Info;

void init_global_info(Global_Info *info, ...);
void update_global_info(Global_Info *info);

void schedule_sjf(Job* jobs, int n);
void schedule_rr(Job* jobs, int n, int time_quatum);
void schedule_mlfq(Job* jobs, int n);


#endif //SCHEDULER_H
