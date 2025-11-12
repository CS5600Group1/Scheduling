//
// Created by zheng on 2025/11/11.
//

#ifndef QUEUE_H
#define QUEUE_H

#include "job.h"

// Queue types
typedef enum {
    QUEUE_FIFO,              // First-In-First-Out (for Round Robin, I/O queue)
    QUEUE_SJF,               // Shortest Job First (priority by remaining time)
    QUEUE_PRIORITY           // Priority queue (for MLFQ levels)
} QueueType;

// Queue Node - wraps a Job pointer with a next pointer
typedef struct QueueNode {
    Job *job;                    // Pointer to the job
    int remaining_time;          // Remaining service time (for SJF scheduling)
    struct QueueNode *next;      // Pointer to next node
} QueueNode;

// Queue structure (implemented as linked list)
typedef struct Queue {
    QueueNode *head;         // Pointer to first node in queue
    QueueNode *tail;         // Pointer to last node in queue
    int size;                // Number of jobs in queue
    QueueType type;          // Type of queue (affects insertion order)
} Queue;

// Function declarations

// Queue management
Queue* create_queue(QueueType type);
void destroy_queue(Queue *queue);
void clear_queue(Queue *queue);  // Remove all jobs but don't destroy them

// Queue operations
void enqueue(Queue *queue, Job *job, int remaining_time);
Job* dequeue(Queue *queue);
Job* peek(Queue *queue);
int is_empty(Queue *queue);
int queue_size(Queue *queue);

// Utility functions
void remove_job(Queue *queue, Job *job);  // Remove specific job from queue
Job* find_job_by_pid(Queue *queue, int pid);  // Find job by PID
void print_queue(Queue *queue);  // For debugging
void update_remaining_time(Queue *queue, Job *job, int new_remaining_time);  // Update remaining time for a job in queue

#endif //QUEUE_H
