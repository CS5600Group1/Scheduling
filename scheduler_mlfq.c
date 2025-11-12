//
// Multi-Level Feedback Queue Scheduler
// Implementation File
//

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "include/scheduler_mlfq.h"
#include "include/scheduler.h"

// Helper function to get time slice for a queue level
static int get_time_slice(int queue_level) {
    switch (queue_level) {
        case 0: return MLFQ_TIME_SLICE_Q0;
        case 1: return MLFQ_TIME_SLICE_Q1;
        case 2: return MLFQ_TIME_SLICE_Q2;
        default: return MLFQ_TIME_SLICE_Q2;
    }
}

// Main MLFQ scheduler
void schedule_mlfq(Job **jobs, int num_jobs) {
    printf("MLFQ scheduler is not implemented yet\n");
}
