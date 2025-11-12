//
// Created by zheng on 2025/11/11.
//

#include <stdlib.h>
#include <stdio.h>
#include "../include/queue.h"

// Create a new queue node
static QueueNode* create_node(Job *job, int remaining_time) {
    QueueNode *node = (QueueNode*)malloc(sizeof(QueueNode));
    if (node == NULL) {
        return NULL;
    }
    node->job = job;
    node->remaining_time = remaining_time;
    node->next = NULL;
    return node;
}

// Create a new queue
Queue* create_queue(QueueType type) {
    Queue *queue = (Queue*)malloc(sizeof(Queue));
    if (queue == NULL) {
        return NULL;
    }

    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    queue->type = type;

    return queue;
}

// Destroy queue and free all nodes (but not the jobs themselves)
void destroy_queue(Queue *queue) {
    if (queue == NULL) {
        return;
    }

    // Free all nodes in the queue
    QueueNode *current = queue->head;
    while (current != NULL) {
        QueueNode *next = current->next;
        free(current);
        current = next;
    }

    free(queue);
}

// Clear queue - remove all nodes but don't destroy the queue itself
void clear_queue(Queue *queue) {
    if (queue == NULL) {
        return;
    }

    QueueNode *current = queue->head;
    while (current != NULL) {
        QueueNode *next = current->next;
        free(current);
        current = next;
    }

    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
}

// Enqueue a job based on queue type
void enqueue(Queue *queue, Job *job, int remaining_time) {
    if (queue == NULL || job == NULL) {
        return;
    }

    QueueNode *node = create_node(job, remaining_time);
    if (node == NULL) {
        return;
    }

    // Empty queue case
    if (queue->head == NULL) {
        queue->head = node;
        queue->tail = node;
        queue->size = 1;
        return;
    }

    switch (queue->type) {
        case QUEUE_FIFO:
            // Add to end of queue (FIFO)
            queue->tail->next = node;
            queue->tail = node;
            queue->size++;
            break;

        case QUEUE_SJF:
            // Insert based on remaining time (shortest first)
            // If tie, use PID (lower PID first)
            {
                QueueNode *current = queue->head;
                QueueNode *prev = NULL;

                // Find insertion point
                while (current != NULL) {
                    if (node->remaining_time < current->remaining_time ||
                        (node->remaining_time == current->remaining_time &&
                         job->PID < current->job->PID)) {
                        break;
                    }
                    prev = current;
                    current = current->next;
                }

                // Insert at beginning
                if (prev == NULL) {
                    node->next = queue->head;
                    queue->head = node;
                } else {
                    // Insert in middle or end
                    prev->next = node;
                    node->next = current;
                    if (current == NULL) {
                        queue->tail = node;  // Update tail if inserted at end
                    }
                }
                queue->size++;
            }
            break;

        case QUEUE_PRIORITY:
            // Insert based on priority (lower number = higher priority)
            // If tie, use PID (lower PID first)
            {
                QueueNode *current = queue->head;
                QueueNode *prev = NULL;

                // Find insertion point
                while (current != NULL) {
                    if (job->priority < current->job->priority ||
                        (job->priority == current->job->priority &&
                         job->PID < current->job->PID)) {
                        break;
                    }
                    prev = current;
                    current = current->next;
                }

                // Insert at beginning
                if (prev == NULL) {
                    node->next = queue->head;
                    queue->head = node;
                } else {
                    // Insert in middle or end
                    prev->next = node;
                    node->next = current;
                    if (current == NULL) {
                        queue->tail = node;  // Update tail if inserted at end
                    }
                }
                queue->size++;
            }
            break;
    }
}

// Dequeue (remove and return first job)
Job* dequeue(Queue *queue) {
    if (queue == NULL || queue->head == NULL) {
        return NULL;
    }

    QueueNode *node = queue->head;
    Job *job = node->job;

    queue->head = node->next;

    // If queue is now empty, update tail
    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    queue->size--;
    free(node);

    return job;
}

// Peek at first job without removing
Job* peek(Queue *queue) {
    if (queue == NULL || queue->head == NULL) {
        return NULL;
    }
    return queue->head->job;
}

// Check if queue is empty
int is_empty(Queue *queue) {
    if (queue == NULL) {
        return 1;  // Treat NULL queue as empty
    }
    return queue->head == NULL;
}

// Get queue size
int queue_size(Queue *queue) {
    if (queue == NULL) {
        return 0;
    }
    return queue->size;
}

// Remove a specific job from queue
void remove_job(Queue *queue, Job *job) {
    if (queue == NULL || job == NULL || queue->head == NULL) {
        return;
    }

    QueueNode *current = queue->head;
    QueueNode *prev = NULL;

    // Search for the job
    while (current != NULL && current->job != job) {
        prev = current;
        current = current->next;
    }

    // Job not found
    if (current == NULL) {
        return;
    }

    // Remove node from list
    if (prev == NULL) {
        // Removing head
        queue->head = current->next;
        if (queue->head == NULL) {
            queue->tail = NULL;
        }
    } else {
        prev->next = current->next;
        if (current->next == NULL) {
            queue->tail = prev;  // Update tail if removed last element
        }
    }

    queue->size--;
    free(current);
}

// Find job by PID
Job* find_job_by_pid(Queue *queue, int pid) {
    if (queue == NULL) {
        return NULL;
    }

    QueueNode *current = queue->head;
    while (current != NULL) {
        if (current->job->PID == pid) {
            return current->job;
        }
        current = current->next;
    }

    return NULL;
}

// Update remaining time for a job in the queue
void update_remaining_time(Queue *queue, Job *job, int new_remaining_time) {
    if (queue == NULL || job == NULL) {
        return;
    }

    QueueNode *current = queue->head;
    while (current != NULL) {
        if (current->job == job) {
            current->remaining_time = new_remaining_time;
            return;
        }
        current = current->next;
    }
}

// Print queue contents (for debugging)
void print_queue(Queue *queue) {
    if (queue == NULL) {
        printf("Queue is NULL\n");
        return;
    }

    const char *type_str[] = {"FIFO", "SJF", "PRIORITY"};
    printf("Queue (size=%d, type=%s): ", queue->size, type_str[queue->type]);

    if (queue->head == NULL) {
        printf("EMPTY\n");
        return;
    }

    QueueNode *current = queue->head;
    while (current != NULL) {
        printf("[PID:%d,RT:%d] ", current->job->PID, current->remaining_time);
        current = current->next;
    }
    printf("\n");
}
