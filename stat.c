#include "scheduler.h"
#include <stdio.h> // For printf
#include <stdlib.h> // For exit
#include "include/job.h"    // For Job struct

/**
 * Helper function: Prints the final statistics report
 * Strictly follows PDF/Image format [cite: 1]
 */
static void print_statistics(Job** all_jobs, int n, int total_sim_time, const Global_Info* info) {
    // Print table header
    printf("Job#   | Total time in ready to run state | Total time in sleeping on I/O state | Total time in system\n");
    printf("=======+==================================+=====================================+======================\n");
    
    // Loop and print each job
    for (int i = 0; i < n; i++) {
        Job* job = all_jobs[i];
        // Format: "pid" + PID number
        printf("pid%-4d| %-32d | %-35d | %-20d\n", 
               job->PID, 
               job->info.ready,
               job->info.sleep,
               job->info.total);
    }
    
    // Print separator line
    printf("=======+==================================+=====================================+======================\n\n");

    // Print summary
    printf("Total simulation run time: %d\n", total_sim_time);
    printf("Total number of jobs: %d\n", info->total_number_of_job);
    printf("Shortest job completion time: %d\n", info->shortest_job_completion_time);
    printf("Longest job completion time: %d\n", info->longest_job_completion_time);
    
    // Ensure averages are printed as floats
    printf("Average job completion time: %.2f\n", info->average_completion);
    printf("Average time in ready queue: %.2f\n", info->average_ready);
    printf("Average time sleeping on I/O state: %.2f\n", info->average_sleep);
}

/**
 * Initializes the Global_Info struct
 */
void init_global_info(Global_Info* info) {
    if (info == NULL) return;
    
    info->total_simulate_time = 0;
    info->total_number_of_job = 0;
    info->shortest_job_completion_time = -1; // -1 means not set
    info->longest_job_completion_time = -1;  // -1 means not set
    info->average_completion = 0.0;
    info->average_ready = 0.0;
    info->average_sleep = 0.0;
}

/**
 * The "update" function:
 * Called at simulation end to calculate and print all stats
 */
void calculate_and_print_final_stats(Global_Info* info, Job** all_jobs, int n, int total_sim_time) {
    if (info == NULL || all_jobs == NULL || n == 0) {
        // Print empty report if no jobs
        print_statistics(all_jobs, n, total_sim_time, info);
        return;
    }

    info->total_simulate_time = total_sim_time;
    info->total_number_of_job = n;

    double total_completion = 0;
    double total_ready = 0;
    double total_sleep = 0;

    for (int i = 0; i < n; i++) {
        Job* job = all_jobs[i];
        int time_in_system = job->info.total; // Assumes info.total is "time in system"
        
        total_completion += time_in_system;
        total_ready += job->info.ready;
        total_sleep += job->info.sleep;

        if (info->shortest_job_completion_time == -1 || time_in_system < info->shortest_job_completion_time) {
            info->shortest_job_completion_time = time_in_system;
        }
        if (info->longest_job_completion_time == -1 || time_in_system > info->longest_job_completion_time) {
            info->longest_job_completion_time = time_in_system;
        }
    }

    info->average_completion = (n > 0) ? (total_completion / n) : 0;
    info->average_ready = (n > 0) ? (total_ready / n) : 0;
    info->average_sleep = (n > 0) ? (total_sleep / n) : 0;

    // After calculation, call the private print function
    print_statistics(all_jobs, n, total_sim_time, info);
}
