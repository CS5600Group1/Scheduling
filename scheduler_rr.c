#include <stdio.h>
#include <stdlib.h>

#include "include/scheduler.h"
#include "include/job.h"
#include "include/queue.h"
#include "include/utils.h"

// --- Context and State for Round Robin ---

typedef enum {
    RR_JOB_STATE_NEW = 0,
    RR_JOB_STATE_READY,
    RR_JOB_STATE_RUNNING,
    RR_JOB_STATE_IO,
    RR_JOB_STATE_DONE
} RRJobState;

typedef struct {
    Job *job;
    int remaining_time;
    int time_slice_used;
    RRJobState state;
} RRJobContext;


// --- Helper Functions (Patterned after SJF/MLFQ) ---

/**
 * @brief qsort comparison function. Sorts by arrival time, then PID.
 */
static int compare_jobs_for_sort(const void *a, const void *b) {
    RRJobContext *ctxA = (RRJobContext *)a;
    RRJobContext *ctxB = (RRJobContext *)b;

    if (ctxA->job->arrival < ctxB->job->arrival) return -1;
    if (ctxA->job->arrival > ctxB->job->arrival) return 1;
    
    // Arrival times are equal, use PID for tie-breaking
    if (ctxA->job->PID < ctxB->job->PID) return -1;
    if (ctxA->job->PID > ctxB->job->PID) return 1;
    
    return 0;
}

/**
 * @brief Helper to find the context for a given job pointer.
 */
static RRJobContext *find_context(RRJobContext *contexts, int count, Job *job) {
    if (job == NULL) {
        return NULL;
    }
    for (int i = 0; i < count; ++i) {
        if (contexts[i].job == job) {
            return &contexts[i];
        }
    }
    return NULL;
}

/**
 * @brief Handles I/O completions.
 * Iterates I/O queue, calls IO_complete(), and moves finished jobs to ready_queue.
 * Handles PID tie-breaking for simultaneous completions.
 */
static void process_io_queue(Queue* io_queue, Queue* ready_queue, RRJobContext* contexts, int n) {
    int io_q_size = queue_size(io_queue);
    if (io_q_size == 0) {
        return;
    }

    // Temp list for jobs that completed I/O this tick
    Job* completed_jobs[io_q_size];
    int completed_count = 0;
    
    Queue* temp_io_queue = create_queue(QUEUE_FIFO); 

    // 1. Iterate I/O queue, check for completions
    for (int i = 0; i < io_q_size; i++) {
        Job* job = dequeue(io_queue);
        RRJobContext* ctx = find_context(contexts, n, job);
        
        // Call sleep() before IO_complete()
        // Job was in I/O queue for a tick
        if (ctx) sleep(ctx->job);

        if (IO_complete() == 1) { // I/O complete
            completed_jobs[completed_count++] = job;
        } else { // I/O not complete
            enqueue(temp_io_queue, job, ctx ? ctx->remaining_time : 0);
        }
    }

    // 2. Re-enqueue pending I/O jobs
    while (!is_empty(temp_io_queue)) {
        enqueue(io_queue, dequeue(temp_io_queue), 0);
    }
    destroy_queue(temp_io_queue);

    // 3. Handle ties: sort completed jobs by PID
    if (completed_count > 1) {
        for (int i = 1; i < completed_count; i++) {
            Job* key = completed_jobs[i];
            int j = i - 1;
            while (j >= 0 && completed_jobs[j]->PID > key->PID) {
                completed_jobs[j + 1] = completed_jobs[j];
                j = j - 1;
            }
            completed_jobs[j + 1] = key;
        }
    }

    // 4. Enqueue completed jobs to ready queue (by PID)
    for (int i = 0; i < completed_count; i++) {
        RRJobContext* ctx = find_context(contexts, n, completed_jobs[i]);
        if (ctx) {
            ctx->state = RR_JOB_STATE_READY;
            ctx->time_slice_used = 0; // Reset time slice on I/O completion
            enqueue(ready_queue, ctx->job, ctx->remaining_time);
        }
    }
}

/**
 * @brief Iterates ready queue to accumulate wait time.
 * (I/O sleep time is handled in process_io_queue)
 */
static void accumulate_ready_queue_stats(Queue* ready_queue) {
    // Accumulate wait time for jobs in ready queue
    QueueNode *node = ready_queue->head;
    while (node != NULL) {
        wait(node->job);
        node = node->next;
    }
}


// --- Main Round Robin Scheduler Function ---

void schedule_rr(Job** jobs, int n, int time_quantum) {
    if (jobs == NULL || n <= 0) {
        return;
    }

    // 1. Initialization
    RRJobContext *contexts = (RRJobContext*)malloc(sizeof(RRJobContext) * n);
    if (contexts == NULL) {
        fprintf(stderr, "RR scheduler: failed to allocate job context array\n");
        return;
    }

    for (int i = 0; i < n; ++i) {
        contexts[i].job = jobs[i];
        contexts[i].remaining_time = (jobs[i] != NULL) ? jobs[i]->service : 0;
        contexts[i].state = RR_JOB_STATE_NEW;
        contexts[i].time_slice_used = 0;
        // Ensure Job's info struct is initialized
        init_Job(contexts[i].job, contexts[i].job->PID, contexts[i].job->arrival, contexts[i].job->service, contexts[i].job->priority);
    }

    // Sort contexts by arrival time / PID (like MLFQ)
    qsort(contexts, n, sizeof(RRJobContext), compare_jobs_for_sort);

    Queue *ready_queue = create_queue(QUEUE_FIFO);
    Queue *io_queue = create_queue(QUEUE_FIFO);
    Global_Info stats_info;
    
    // init_global_info(&stats_info);
    init_clock();
    os_srand(1); // Required by PDF for determinism

    int completed_jobs = 0;
    int next_job_index = 0; // Tracks next job in sorted context array
    RRJobContext *current_job_ctx = NULL;
    
    int total_jobs_in_system = 0;

    // 2. Main Simulation Loop
    while (completed_jobs < n) {
        int clock_tick = current_clock();
        
        // Flag: is any job running or waiting?
        int job_running_or_waiting = 0;

        // Step 1: Enqueue new arrivals
        while (next_job_index < n && contexts[next_job_index].job->arrival <= clock_tick) {
            contexts[next_job_index].state = RR_JOB_STATE_READY;
            enqueue(ready_queue, contexts[next_job_index].job, contexts[next_job_index].remaining_time);
            next_job_index++;
            total_jobs_in_system++;
        }
        
        // Step 2: Process I/O completions (Strict PDF order)
        process_io_queue(io_queue, ready_queue, contexts, n);

        // Step 3: Handle running job logic
        if (current_job_ctx != NULL) {
            
            // Check for Time Slice Expiry
            if (current_job_ctx->time_slice_used >= time_quantum) {
                current_job_ctx->state = RR_JOB_STATE_READY;
                enqueue(ready_queue, current_job_ctx->job, current_job_ctx->remaining_time);
                current_job_ctx = NULL;
            }
        }

        // Step 4: Select new job if CPU is idle
        if (current_job_ctx == NULL) {
            Job *next_job = dequeue(ready_queue);
            if (next_job != NULL) {
                current_job_ctx = find_context(contexts, n, next_job);
                current_job_ctx->state = RR_JOB_STATE_RUNNING;
                current_job_ctx->time_slice_used = 0; // Reset time slice
            }
        }

        // Step 5: Accumulate stats for waiting jobs
        accumulate_ready_queue_stats(ready_queue);

        // Step 6: Run the current job
        if (current_job_ctx != NULL) {
            job_running_or_waiting = 1; // Mark CPU as active
            run(current_job_ctx->job);
            current_job_ctx->remaining_time--;
            current_job_ctx->time_slice_used++;

            // Check for Job Completion
            if (current_job_ctx->remaining_time <= 0) {
                current_job_ctx->state = RR_JOB_STATE_DONE;
                //current_job_ctx->job->info.completion_time = clock_tick + 1; // Job completes at tick+1
                //current_job_ctx->job->info.total = current_job_ctx->job->info.completion_time - current_job_ctx->job->arrival;
                completed_jobs++;
                total_jobs_in_system--;
                current_job_ctx = NULL;
            } 
            // Check for I/O Request
            else if (IO_request()) {
                current_job_ctx->state = RR_JOB_STATE_IO;
                enqueue(io_queue, current_job_ctx->job, current_job_ctx->remaining_time);
                current_job_ctx = NULL;
            }
        }
        
        // Check if any job is in I/O queue
        if (!is_empty(io_queue)) {
            job_running_or_waiting = 1;
        }
        
        // PDF idle process check:
        // If no job is running,
        // and no new jobs are arriving,
        // and ready queue is empty,
        // and I/O queue is empty,
        // then exit.
        if (current_job_ctx == NULL && is_empty(ready_queue) && is_empty(io_queue) && next_job_index >= n) {
             break; // All jobs are processed
        }
        
        // Safety break
        if (clock_tick > 200000) { 
            fprintf(stderr, "Error: RR simulation exceeded maximum time limit\n");
            break;
        }

        // Step 7: Advance clock
        next_tick();
    }

    // 3. Finalization
    // Must pass original 'jobs' array to stats
    calculate_and_print_final_stats(&stats_info, jobs, n, current_clock());

    // 4. Cleanup
    destroy_queue(io_queue);
    destroy_queue(ready_queue);
    free(contexts);
}


