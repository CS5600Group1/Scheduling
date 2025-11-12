#include "scheduler.h" // Includes new function declarations
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"     // Using your queue.h
#include "clock.h"     // Using your clock.h
#include "job.h"       // Using your job.h

// --- Private Helper Functions ---

/**
 * Helper function: Prints the final statistics report
 */
// static void print_statistics(...) { ... } // <-- moved to stats.c


/**
 * Helper function: Handles new job arrivals
 * Checks all_jobs list, adds jobs arriving at current_clock
 * to the ready queue, respecting PID order
 */
static int handle_new_arrivals(Job* all_jobs, int n, int* next_job_index, Queue* ready_queue) {
    int current_time = current_clock();
    int new_arrivals = 0;

    // Assumes all_jobs is pre-sorted by arrival time, then PID
    // (Must be done in main.c)
    while (*next_job_index < n && all_jobs[*next_job_index].arrival <= current_time) {
        Job* new_job = &all_jobs[*next_job_index];
        // Note: Assumes job.h has service_remaining field
        new_job->service_remaining = new_job->service;

        // Use your enqueue API. For FIFO, remaining_time param is ignored
        enqueue(ready_queue, new_job, new_job->service_remaining);
        (*next_job_index)++;
        new_arrivals++;
    }
    return new_arrivals;
}

/**
 * Helper function: Handles I/O completions
 * Strictly follows PDF order: IO_complete first, then IO_request
 * Also handles PID tie-breaking for I/O completion
 */
static void handle_io_completions(Queue* io_wait_queue, Queue* ready_queue) {
    int io_q_size = queue_size(io_wait_queue);
    if (io_q_size == 0) {
        return;
    }

    // Temp list for jobs that completed I/O this tick
    Job* completed_jobs[io_q_size];
    int completed_count = 0;

    // Temp queue for jobs with pending I/O
    Queue* temp_io_queue = create_queue(QUEUE_FIFO);

    // 1. Iterate I/O queue
    for (int i = 0; i < io_q_size; i++) {
        Job* job = dequeue(io_wait_queue);

        if (IO_complete() == 1) { // I/O complete
            completed_jobs[completed_count++] = job;
        } else { // I/O not complete
            enqueue(temp_io_queue, job, 0); // r_time doesn't matter
        }
    }

    // 2. Re-enqueue pending I/O jobs
    while (!is_empty(temp_io_queue)) {
        enqueue(io_wait_queue, dequeue(temp_io_queue), 0); // r_time doesn't matter
    }
    destroy_queue(temp_io_queue); // Clean up temp queue

    // 3. Handle ties: sort completed jobs by PID
    if (completed_count > 1) {
        // Simple insertion sort
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
        // Assumes job.h has service_remaining
        enqueue(ready_queue, completed_jobs[i], completed_jobs[i]->service_remaining);
    }
}

/**
 * Helper function: Updates wait stats for all jobs
 * (Called at the end of each tick)
 * Uses your wait() and sleep() functions
 */
static void update_job_stats(Queue* ready_queue, Queue* io_wait_queue) {
    QueueNode* current = ready_queue->head;
    while (current != NULL) {
        wait(current->job); // Call wait function
        current = current->next;
    }

    current = io_wait_queue->head;
    while (current != NULL) {
        sleep(current->job); // Call sleep function
        current = current->next;
    }
}

/**
 * Helper function: Calculates and updates final global stats
 */
// static void calculate_final_stats(...) { ... } // <-- moved to stats.c


// --- Public Scheduler Function ---

/**
 * Round Robin (RR) scheduler simulation
 */
void schedule_rr(Job* all_jobs, int n, int time_quantum) {

    // 1. Initialization
    init_clock();
    os_srand(1); // Set seed as required for determinism

    Global_Info stats_info; // For final statistics
    init_global_info(&stats_info); // <-- Call shared init function

    // Use your create_queue API
    Queue* ready_queue = create_queue(QUEUE_FIFO);
    Queue* io_wait_queue = create_queue(QUEUE_FIFO); // I/O queue is also FIFO

    Job* current_job_on_cpu = NULL;
    int jobs_completed = 0;
    int next_job_index = 0; // Tracks next job in all_jobs array
    int current_time_slice = 0; // Time slice used by current job

    // 2. Main simulation loop
    while (jobs_completed < n) {
        int current_time = current_clock();

        // Step 1: Add new arrivals to ready queue
        handle_new_arrivals(all_jobs, n, &next_job_index, ready_queue);

        // Step 2: Handle I/O completions
        handle_io_completions(io_wait_queue, ready_queue);

        // Step 3: Process the job currently on the CPU
        if (current_job_on_cpu != NULL) {

            // Check if job is finished
            // Assumes job.h has service_remaining
            if (current_job_on_cpu->service_remaining <= 0) {
                // Assumes OutputBlock has completion_time
                current_job_on_cpu->info.completion_time = current_time;
                current_job_on_cpu->info.total = current_time - current_job_on_cpu->arrival;
                jobs_completed++;
                current_job_on_cpu = NULL;
                current_time_slice = 0;
            }
            // Check for I/O request
            else if (IO_request() == 1) {
                enqueue(io_wait_queue, current_job_on_cpu, 0); // r_time doesn't matter
                current_job_on_cpu = NULL;
                current_time_slice = 0;
            }
            // Check if time slice is used up
            else if (current_time_slice >= time_quantum) {
                // Assumes job.h has service_remaining
                enqueue(ready_queue, current_job_on_cpu, current_job_on_cpu->service_remaining);
                current_job_on_cpu = NULL;
                current_time_slice = 0;
            }
        }

        // Step 4: If CPU is idle, select a new job
        if (current_job_on_cpu == NULL) {
            if (!is_empty(ready_queue)) {
                current_job_on_cpu = dequeue(ready_queue);
                current_time_slice = 0;
            }
            // else: CPU is idle
        }

        // Step 5: Run job & update stats
        // Update stats for all waiting jobs
        update_job_stats(ready_queue, io_wait_queue);

        if (current_job_on_cpu != NULL) {
            run(current_job_on_cpu); // Call run function
            current_time_slice++;
        }

        // Step 6: Advance clock
        next_tick();
    }

    // 3. Simulation finished, calculate and print stats
    int final_time = current_clock() - 1; // Loop ticks one last time after completion

    // <-- Old calls replaced
    // calculate_final_stats(all_jobs, n, &stats_info, final_time);
    // print_statistics(all_jobs, n, final_time, &stats_info);

    // <-- Call shared stats function
    calculate_and_print_final_stats(&stats_info, all_jobs, n, final_time);

    // 4. Cleanup
    destroy_queue(ready_queue);
    destroy_queue(io_wait_queue);
}