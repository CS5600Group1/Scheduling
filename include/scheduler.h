//
// Created by zheng on 2025/11/11.
//

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "job.h"

typedef enum {
    SCHED_POLICY_SJF,
    SCHED_POLICY_RR,
    SCHED_POLICY_MLFQ
} SchedulerPolicy;

typedef struct {
    int total_simulate_time;
    int total_number_of_job;
    int shortest_job_completion_time;
    int longest_job_completion_time;
    int average_completion;
    int average_ready;
    int average_sleep;
} Global_Info;

void init_global_info(Global_Info *info);
void calculate_and_print_final_stats(Global_Info* info, Job** all_jobs, int n, int total_sim_time);

void schedule(Job **jobs, int n, SchedulerPolicy policy, int time_quantum);

void schedule_sjf(Job **jobs, int n);
void schedule_rr(Job **jobs, int n, int time_quatum);
void schedule_mlfq(Job **jobs, int n);


#endif //SCHEDULER_H
