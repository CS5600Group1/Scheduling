//
// Created by zheng on 2025/11/12.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "job.h"

void init_OutputBlock(OutputBlock *info) {
    if (info == NULL) {
        return;
    }
    memset(info, 0, sizeof(OutputBlock));
}

void init_Job(Job **job, int pid, int arrival, int service, int priority){
    *job = (Job*)malloc(sizeof(Job));
    if (*job == NULL) {
        fprintf(stderr, "Error: memory allocation failed for Job\n");
        return;
    }

    (*job)->PID = pid;
    (*job)->arrival = arrival;
    (*job)->service = service;
    (*job)->priority = priority;

    init_OutputBlock(&(*job)->info);
}

OutputBlock get_Job_info(const Job *job) {
    OutputBlock info = {0};
    if (job != NULL) {
        info = job->info;
    }
    return info;
}

void wait(Job *job) {
    if (job == NULL) {
        return;
    }
    job->info.ready++;
}

void sleep(Job *job) {
    if (job == NULL) {
        return;
    }
    job->info.sleep++;
    job->info.total++;
}

void run(Job *job) {
    if (job == NULL) {
        return;
    }
    job->info.total++;
}
