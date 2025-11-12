#include <stdio.h>

#include "include/scheduler.h"
#include "include/clock.h"

static void report_invalid_policy(SchedulerPolicy policy) {
    fprintf(stderr, "Error: unsupported scheduler policy (%d)\n", policy);
}

void schedule(Job **jobs, int n, SchedulerPolicy policy, int time_quantum) {

    if (jobs == NULL || n <= 0) {
        fprintf(stderr, "Error: no jobs available for scheduling\n");
        return;
    }

    switch (policy) {
        case SCHED_POLICY_SJF:
            schedule_sjf(jobs, n);
            break;
        case SCHED_POLICY_RR:
            if (time_quantum <= 0) {
                fprintf(stderr, "Error: round-robin requires a positive time quantum\n");
                return;
            }
            schedule_rr(jobs, n, time_quantum);
            break;
        case SCHED_POLICY_MLFQ:
            schedule_mlfq(jobs, n);
            break;
        default:
            report_invalid_policy(policy);
            break;
    }
}

