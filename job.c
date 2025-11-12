//
// Created by zheng on 2025/11/12.
//
#include <stdlib.h>

#include "job.h"
#include "clock.h"

void init_OutputBlock(OutputBlock *info){

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

void run(Job *job){

}