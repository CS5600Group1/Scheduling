//
// Created by zheng on 2025/11/11.
//

#ifndef JOB_H
#define JOB_H

typedef struct {
    int ready;
    int sleep;
    int total;
} OutputBlock;

typedef struct {
    int PID;
    int arrival;
    int service;
    int priority;

    OutputBlock info;
} Job;

void init_OutputBlock(OutputBlock *info);
void init_Job(Job **job, int pid, int arrival, int service, int priority);


OutputBlock get_Job_info(Job *job);
void run(Job *job);

int IO_request();
int IO_complete();

#endif //JOB_H
