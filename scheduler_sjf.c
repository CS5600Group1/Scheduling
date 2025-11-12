#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "include/scheduler.h"
#include "include/queue.h"
#include "include/clock.h"
#include "include/utils.h"

typedef enum {
    JOB_STATE_NEW = 0,
    JOB_STATE_READY,
    JOB_STATE_RUNNING,
    JOB_STATE_IO,
    JOB_STATE_DONE
} SJFJobState;

typedef struct {
    Job *job;
    int remaining_time;
    bool isStarted;
    SJFJobState state;
} SJFJobContext;

static SJFJobContext *find_context(SJFJobContext *contexts, int count, Job *job) {
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

static void enqueue_new_arrivals(SJFJobContext *contexts, int count, int clock_tick, Queue *ready_queue) {
    for (int i = 0; i < count; ++i) {
        if (contexts[i].state == JOB_STATE_NEW && contexts[i].job->arrival <= clock_tick) {
            enqueue(ready_queue, contexts[i].job, contexts[i].remaining_time);
            contexts[i].state = JOB_STATE_READY;
        }
    }
}

static void process_io_queue(Queue *io_queue, Queue *ready_queue, SJFJobContext *contexts, int count) {
    if (io_queue == NULL) {
        return;
    }

    int pending = queue_size(io_queue);
    for (int i = 0; i < pending; ++i) {
        Job *job = dequeue(io_queue);
        if (job == NULL) {
            continue;
        }

        SJFJobContext *ctx = find_context(contexts, count, job);
        if (ctx == NULL) {
            continue;
        }

        sleep(job);
        if (IO_complete()) {
            ctx->state = JOB_STATE_READY;
            enqueue(ready_queue, job, ctx->remaining_time);
        } else {
            enqueue(io_queue, job, ctx->remaining_time);
        }
    }
}

static void accumulate_wait_time(SJFJobContext *contexts, int count) {
    for (int i = 0; i < count; ++i) {
        if (contexts[i].state == JOB_STATE_READY && contexts[i].isStarted == false) {
            wait(contexts[i].job);
        }
    }
}

void schedule_sjf(Job **jobs, int n) {
    if (jobs == NULL || n <= 0) {
        return;
    }

    SJFJobContext *contexts = (SJFJobContext*)malloc(sizeof(SJFJobContext) * n);
    if (contexts == NULL) {
        fprintf(stderr, "SJF scheduler: failed to allocate job context array\n");
        return;
    }

    for (int i = 0; i < n; ++i) {
        contexts[i].job = jobs[i];
        contexts[i].remaining_time = (jobs[i] != NULL) ? jobs[i]->service : 0;
        contexts[i].isStarted = false;
        contexts[i].state = JOB_STATE_NEW;
    }

    Queue *ready_queue = create_queue(QUEUE_SJF);
    if (ready_queue == NULL) {
        fprintf(stderr, "SJF scheduler: failed to create ready queue\n");
        free(contexts);
        return;
    }

    Queue *io_queue = create_queue(QUEUE_FIFO);
    if (io_queue == NULL) {
        fprintf(stderr, "SJF scheduler: failed to create I/O queue\n");
        destroy_queue(ready_queue);
        free(contexts);
        return;
    }

    init_clock();
    int completed_jobs = 0;
    SJFJobContext *current = NULL;

    while (completed_jobs < n) {
        int clock_tick = current_clock();

        enqueue_new_arrivals(contexts, n, clock_tick, ready_queue);
        process_io_queue(io_queue, ready_queue, contexts, n);

        if (current != NULL && current->state == JOB_STATE_RUNNING) {
            Job *head_job = peek(ready_queue);
            SJFJobContext *candidate = find_context(contexts, n, head_job);
            if (candidate != NULL && candidate->remaining_time < current->remaining_time) {
                current->state = JOB_STATE_READY;
                enqueue(ready_queue, current->job, current->remaining_time);
                current = NULL;
            }
        }

        if (current == NULL) {
            Job *next_job = dequeue(ready_queue);
            if (next_job != NULL) {
                current = find_context(contexts, n, next_job);
                if (current != NULL) {
                    current->state = JOB_STATE_RUNNING;
                    current->isStarted = true;
                }
            }
        }

        accumulate_wait_time(contexts, n);

        if (current != NULL && current->state == JOB_STATE_RUNNING) {
            run(current->job);
            current->remaining_time--;

            if (current->remaining_time <= 0) {
                current->state = JOB_STATE_DONE;
                completed_jobs++;
                current = NULL;
            } else if (IO_request()) {
                current->state = JOB_STATE_IO;
                enqueue(io_queue, current->job, current->remaining_time);
                current = NULL;
            }
        }

        next_tick();

        if (completed_jobs >= n) {
            break;
        }
    }

    destroy_queue(io_queue);
    destroy_queue(ready_queue);
    free(contexts);
}

