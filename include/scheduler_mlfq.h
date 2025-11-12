//
// Multi-Level Feedback Queue Scheduler
// Header File
//

#ifndef SCHEDULER_MLFQ_H
#define SCHEDULER_MLFQ_H

#include "job.h"
#include "queue.h"

// MLFQ Configuration
#define MLFQ_NUM_QUEUES 3
#define MLFQ_TIME_SLICE_Q0 8
#define MLFQ_TIME_SLICE_Q1 16
#define MLFQ_TIME_SLICE_Q2 32
#define MLFQ_BOOST_INTERVAL 100

// Statistics structure
typedef struct {
    int total_jobs;
    int total_simulation_time;
    int shortest_job_time;
    int longest_job_time;
    int total_ready_time;
    int total_sleep_time;
} Statistics;

// MLFQ structure to track job state
typedef struct {
    int current_queue_level;    // Current queue level (0-2)
    int time_slice_used;        // Time used in current time slice
    int total_time_in_queue[MLFQ_NUM_QUEUES];  // Time spent in each queue
} MLFQJobState;

// MLFQ scheduler function
void run_mlfq_scheduler(Job **jobs, int num_jobs);

#endif //SCHEDULER_MLFQ_H
