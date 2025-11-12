#include <stdio.h>

#include "include/scheduler.h"
#include "include/clock.h"

static void report_invalid_policy(SchedulerPolicy policy) {
    fprintf(stderr, "Error: unsupported scheduler policy (%d)\n", policy);
}

static void print_summary(Job **jobs, int n) {
    printf("Job#      | Total time         | Total time         | Total time         |\n");
    printf("          | in ready to run    | in sleeping on     | in system          |\n");
    printf("          | state              | I/O state          |                    |\n");
    printf("==========+====================+====================+====================+\n");

    for (int i = 0; i < n; ++i) {
        Job *job = jobs[i];
        if (job == NULL) {
            continue;
        }
        OutputBlock info = get_Job_info(job);
        printf("%-10d| %-20d| %-20d| %-20d|\n",
               job->PID,
               info.ready,
               info.sleep,
               info.total);
    }
    printf("\n");
}

void schedule(Job **jobs, int n, SchedulerPolicy policy, int time_quantum) {

    if (jobs == NULL || n <= 0) {
        fprintf(stderr, "Error: no jobs available for scheduling\n");
        return;
    }

#if defined(BUILD_SJF_ONLY)
    if (policy != SCHED_POLICY_SJF) {
        fprintf(stderr, "Error: this binary only supports SJF policy.\n");
        return;
    }
    schedule_sjf(jobs, n);
#elif defined(BUILD_RR_ONLY)
    if (policy != SCHED_POLICY_RR) {
        fprintf(stderr, "Error: this binary only supports RR policy.\n");
        return;
    }
    if (time_quantum <= 0) {
        fprintf(stderr, "Error: round-robin requires a positive time quantum\n");
        return;
    }
    schedule_rr(jobs, n, time_quantum);
#elif defined(BUILD_MLFQ_ONLY)
    if (policy != SCHED_POLICY_MLFQ) {
        fprintf(stderr, "Error: this binary only supports MLFQ policy.\n");
        return;
    }
    schedule_mlfq(jobs, n);
#else
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
#endif

    print_summary(jobs, n);
}

