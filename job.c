//
// Created by zheng on 2025/11/12.
//
#include <stdlib.h>
#include <string.h>

#include "job.h"
#include "clock.h"

void init_OutputBlock(OutputBlock *info){
    memset(block, 0, sizeof(OutputBlock));
}

void init_Job(Job **job, int pid, int arrival, int service, int priority){
    *job = (Job*)malloc(sizeof(Job));
    if (*job == NULL) {
        fprintf(stderr, "Error: memory allocation failed for Job\n");
        exit(1);
    }

    (*job)->pid = pid;
    (*job)->arrival = arrival;
    (*job)->service = service;
    (*job)->priority = priority;

    init_OutputBlock(&(*job)->info);
}

OutputBlock get_Job_info(Job *job){
    OutputBlock info = job->info;
    return info;
}

void wait(Job *job){
    job->info.ready++;
}

void sleep(Job *job){
    job->info.sleep++;
}

void run(Job *job){
   job->info.total++;
}
