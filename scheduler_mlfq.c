//
// Multi-Level Feedback Queue Scheduler
// Implementation File
//

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "include/scheduler_mlfq.h"
#include "include/scheduler.h"
#include "include/utils.h"
#include "include/clock.h"

// Helper function to initialize statistics
static void init_statistics(Statistics *stats) {
    stats->total_jobs = 0;
    stats->total_simulation_time = 0;
    stats->shortest_job_time = INT_MAX;
    stats->longest_job_time = 0;
    stats->total_ready_time = 0;
    stats->total_sleep_time = 0;
}

// Helper function to update statistics when a job completes
static void update_statistics(Statistics *stats, Job *job, int completion_time) {
    stats->total_jobs++;
    int turnaround = completion_time - job->arrival;
    if (turnaround < stats->shortest_job_time) {
        stats->shortest_job_time = turnaround;
    }
    if (turnaround > stats->longest_job_time) {
        stats->longest_job_time = turnaround;
    }
    stats->total_ready_time += job->info.ready;
    stats->total_sleep_time += job->info.sleep;
}

// Helper function to print job information
static void print_job_info(Job *job) {
    printf("%-10d| %-19d| %-19d| %-19d|\n",
           job->PID, job->info.ready, job->info.sleep, job->info.total);
}

// Helper function to print statistics
static void print_statistics(Statistics *stats) {
    printf("==========+====================+====================+====================+\n");
    if (stats->total_jobs > 0) {
        printf("Avg       | %-19d| N/A                | N/A                |\n",
               stats->total_ready_time / stats->total_jobs);
    }
    printf("\n");
    printf("Total simulation time: %d\n", stats->total_simulation_time);
    if (stats->total_jobs > 0) {
        printf("Shortest job completion time: %d\n", stats->shortest_job_time);
        printf("Longest job completion time: %d\n", stats->longest_job_time);
    }
}

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
void run_mlfq_scheduler(Job **jobs, int num_jobs) {
    if (jobs == NULL || num_jobs == 0) {
        printf("No jobs to schedule.\n");
        return;
    }

    os_srand(1);
    int clock = 0;
    int next_job_index = 0;  // Index of next job to arrive
    int boost_counter = 0;   // Counter for priority boost

    // Create 3 MLFQ queues (all FIFO, Round Robin within each level)
    Queue *mlfq[MLFQ_NUM_QUEUES];
    for (int i = 0; i < MLFQ_NUM_QUEUES; i++) {
        mlfq[i] = create_queue(QUEUE_FIFO);
    }
    Queue *io_queue = create_queue(QUEUE_FIFO);

    // Track MLFQ state for each job
    MLFQJobState *job_states = (MLFQJobState*)malloc(num_jobs * sizeof(MLFQJobState));
    for (int i = 0; i < num_jobs; i++) {
        job_states[i].current_queue_level = 0;  // Start at highest priority
        job_states[i].time_slice_used = 0;
        for (int j = 0; j < MLFQ_NUM_QUEUES; j++) {
            job_states[i].total_time_in_queue[j] = 0;
        }
    }

    Job *current_job = NULL;
    int current_job_index = -1;
    int current_time_slice = 0;

    Statistics stats;
    init_statistics(&stats);

    // Sort jobs by arrival time
    for (int i = 0; i < num_jobs - 1; i++) {
        for (int j = 0; j < num_jobs - i - 1; j++) {
            if (jobs[j]->arrival > jobs[j + 1]->arrival ||
                (jobs[j]->arrival == jobs[j + 1]->arrival && jobs[j]->PID > jobs[j + 1]->PID)) {
                Job *temp = jobs[j];
                jobs[j] = jobs[j + 1];
                jobs[j + 1] = temp;
                // Swap states too
                MLFQJobState temp_state = job_states[j];
                job_states[j] = job_states[j + 1];
                job_states[j + 1] = temp_state;
            }
        }
    }

    // Main scheduling loop
    while (1) {
        // Rule 5: Priority boost every MLFQ_BOOST_INTERVAL time units
        if (boost_counter >= MLFQ_BOOST_INTERVAL) {
            // Move all jobs from all queues to queue 0
            for (int level = 1; level < MLFQ_NUM_QUEUES; level++) {
                while (!is_empty(mlfq[level])) {
                    Job *job = dequeue(mlfq[level]);
                    // Find job index
                    for (int i = 0; i < num_jobs; i++) {
                        if (jobs[i] == job) {
                            job_states[i].current_queue_level = 0;
                            job_states[i].time_slice_used = 0;
                            break;
                        }
                    }
                    int remaining = job->service - job->info.total;
                    enqueue(mlfq[0], job, remaining);
                }
            }

            // If current job is not in queue 0, move it too
            if (current_job != NULL && current_job_index >= 0) {
                if (job_states[current_job_index].current_queue_level > 0) {
                    job_states[current_job_index].current_queue_level = 0;
                    job_states[current_job_index].time_slice_used = 0;
                }
            }

            boost_counter = 0;
        }

        // Step 1: Add new incoming jobs to highest priority queue (Rule 3)
        while (next_job_index < num_jobs && jobs[next_job_index]->arrival == clock) {
            Job *new_job = jobs[next_job_index];
            job_states[next_job_index].current_queue_level = 0;  // Rule 3
            job_states[next_job_index].time_slice_used = 0;
            int remaining = new_job->service;
            enqueue(mlfq[0], new_job, remaining);
            next_job_index++;
        }

        // Step 2: Check I/O completions (in FIFO order)
        QueueNode *io_node = io_queue->head;
        QueueNode *prev_io_node = NULL;
        while (io_node != NULL) {
            if (IO_complete()) {
                Job *completed_io_job = io_node->job;
                QueueNode *next = io_node->next;

                // Find job index
                int job_idx = -1;
                for (int i = 0; i < num_jobs; i++) {
                    if (jobs[i] == completed_io_job) {
                        job_idx = i;
                        break;
                    }
                }

                // Remove from I/O queue
                if (prev_io_node == NULL) {
                    io_queue->head = next;
                } else {
                    prev_io_node->next = next;
                }
                if (next == NULL) {
                    io_queue->tail = prev_io_node;
                }
                io_queue->size--;
                free(io_node);

                // Rule 4: Job didn't use full time slice (I/O), keep same priority
                if (job_idx >= 0) {
                    int level = job_states[job_idx].current_queue_level;
                    job_states[job_idx].time_slice_used = 0;  // Reset time slice
                    int remaining = completed_io_job->service - completed_io_job->info.total;
                    enqueue(mlfq[level], completed_io_job, remaining);
                }

                io_node = next;
            } else {
                prev_io_node = io_node;
                io_node = io_node->next;
            }
        }

        // Step 3: Check if current job's time slice expired
        if (current_job != NULL && current_job_index >= 0) {
            int level = job_states[current_job_index].current_queue_level;
            int time_slice = get_time_slice(level);

            if (job_states[current_job_index].time_slice_used >= time_slice) {
                // Time slice expired, put back in queue
                int remaining = current_job->service - current_job->info.total;
                enqueue(mlfq[level], current_job, remaining);
                current_job = NULL;
                current_job_index = -1;
            }
        }

        // Step 4: If no current job, select next from highest priority non-empty queue (Rule 1 & 2)
        if (current_job == NULL) {
            for (int level = 0; level < MLFQ_NUM_QUEUES; level++) {
                if (!is_empty(mlfq[level])) {
                    current_job = dequeue(mlfq[level]);

                    // Find job index
                    for (int i = 0; i < num_jobs; i++) {
                        if (jobs[i] == current_job) {
                            current_job_index = i;
                            break;
                        }
                    }

                    current_time_slice = 0;
                    break;
                }
            }
        }

        // Step 5: Run current job
        if (current_job != NULL && current_job_index >= 0) {
            run(current_job);
            job_states[current_job_index].time_slice_used++;
            current_time_slice++;

            // Check if job is complete
            if (current_job->info.total >= current_job->service) {
                // Job completed
                update_statistics(&stats, current_job, clock + 1);
                current_job = NULL;
                current_job_index = -1;
            } else {
                // Check for I/O request
                if (IO_request()) {
                    // Move to I/O queue (Rule 4: didn't use full slice, keep priority)
                    int remaining = current_job->service - current_job->info.total;
                    enqueue(io_queue, current_job, remaining);
                    current_job = NULL;
                    current_job_index = -1;
                } else if (job_states[current_job_index].time_slice_used >= get_time_slice(job_states[current_job_index].current_queue_level)) {
                    // Rule 4: Used full time slice, demote to lower queue
                    int current_level = job_states[current_job_index].current_queue_level;
                    if (current_level < MLFQ_NUM_QUEUES - 1) {
                        job_states[current_job_index].current_queue_level++;
                    }
                    job_states[current_job_index].time_slice_used = 0;

                    int remaining = current_job->service - current_job->info.total;
                    enqueue(mlfq[job_states[current_job_index].current_queue_level], current_job, remaining);
                    current_job = NULL;
                    current_job_index = -1;
                }
            }
        }

        // Step 6: Update waiting times for jobs in all queues
        for (int level = 0; level < MLFQ_NUM_QUEUES; level++) {
            QueueNode *node = mlfq[level]->head;
            while (node != NULL) {
                wait(node->job);
                node = node->next;
            }
        }

        QueueNode *node = io_queue->head;
        while (node != NULL) {
            sleep(node->job);
            node = node->next;
        }

        // Increment clock and boost counter
        clock++;
        boost_counter++;

        // Check termination condition
        int all_queues_empty = 1;
        for (int level = 0; level < MLFQ_NUM_QUEUES; level++) {
            if (!is_empty(mlfq[level])) {
                all_queues_empty = 0;
                break;
            }
        }

        if (current_job == NULL && all_queues_empty &&
            is_empty(io_queue) && next_job_index >= num_jobs) {
            break;
        }

        // Safety check
        if (clock > 100000) {
            printf("Error: Simulation exceeded maximum time limit\n");
            break;
        }
    }

    stats.total_simulation_time = clock;

    // Cleanup
    for (int i = 0; i < MLFQ_NUM_QUEUES; i++) {
        destroy_queue(mlfq[i]);
    }
    destroy_queue(io_queue);
    free(job_states);
}

// Wrapper function to match scheduler.h interface
void schedule_mlfq(Job **jobs, int n) {
    run_mlfq_scheduler(jobs, n);
}
