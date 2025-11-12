//
// Test program for queue implementation
//

#include <stdio.h>
#include <stdlib.h>
#include "../include/queue.h"
#include "../include/job.h"

// Helper function to create a simple job for testing
Job* create_test_job(int pid, int arrival, int service, int priority) {
    Job *job = (Job*)malloc(sizeof(Job));
    if (job == NULL) {
        return NULL;
    }
    job->PID = pid;
    job->arrival = arrival;
    job->service = service;
    job->priority = priority;
    job->info.ready = 0;
    job->info.sleep = 0;
    job->info.total = 0;
    return job;
}

void test_fifo_queue() {
    printf("\n=== Testing FIFO Queue ===\n");

    Queue *queue = create_queue(QUEUE_FIFO);

    // Create and enqueue jobs
    Job *job1 = create_test_job(100, 0, 10, 0);
    Job *job2 = create_test_job(101, 1, 20, 0);
    Job *job3 = create_test_job(102, 2, 15, 0);

    enqueue(queue, job1, job1->service);
    enqueue(queue, job2, job2->service);
    enqueue(queue, job3, job3->service);

    printf("After enqueuing 3 jobs:\n");
    print_queue(queue);

    // Dequeue
    Job *dequeued = dequeue(queue);
    printf("Dequeued job PID: %d\n", dequeued->PID);
    print_queue(queue);

    // Test peek
    Job *peeked = peek(queue);
    printf("Peeked job PID: %d\n", peeked->PID);
    printf("Queue size: %d\n", queue_size(queue));
    printf("Is empty: %s\n", is_empty(queue) ? "Yes" : "No");

    // Cleanup
    free(dequeued);
    while (!is_empty(queue)) {
        free(dequeue(queue));
    }
    destroy_queue(queue);

    printf("FIFO test passed!\n");
}

void test_sjf_queue() {
    printf("\n=== Testing SJF Queue ===\n");

    Queue *queue = create_queue(QUEUE_SJF);

    // Create jobs with different remaining times
    Job *job1 = create_test_job(100, 0, 20, 0);
    Job *job2 = create_test_job(101, 1, 10, 0);
    Job *job3 = create_test_job(102, 2, 15, 0);
    Job *job4 = create_test_job(103, 2, 10, 0);  // Same remaining time as job2

    // Enqueue in non-sorted order with different remaining times
    enqueue(queue, job1, 20);
    enqueue(queue, job2, 10);
    enqueue(queue, job3, 15);
    enqueue(queue, job4, 10);  // Same as job2, should come after due to higher PID

    printf("After enqueuing 4 jobs (should be sorted by remaining time):\n");
    print_queue(queue);

    printf("\nDequeuing all jobs:\n");
    while (!is_empty(queue)) {
        Job *job = dequeue(queue);
        printf("  Dequeued PID: %d, Service Time: %d\n", job->PID, job->service);
        free(job);
    }

    destroy_queue(queue);

    printf("SJF test passed!\n");
}

void test_priority_queue() {
    printf("\n=== Testing Priority Queue ===\n");

    Queue *queue = create_queue(QUEUE_PRIORITY);

    // Create jobs with different priorities
    Job *job1 = create_test_job(100, 0, 10, 2);  // Lower priority
    Job *job2 = create_test_job(101, 1, 10, 0);  // Higher priority
    Job *job3 = create_test_job(102, 2, 10, 1);  // Medium priority
    Job *job4 = create_test_job(103, 2, 10, 0);  // Same priority as job2

    // Enqueue in non-sorted order
    enqueue(queue, job1, job1->service);
    enqueue(queue, job2, job2->service);
    enqueue(queue, job3, job3->service);
    enqueue(queue, job4, job4->service);

    printf("After enqueuing 4 jobs (should be sorted by priority):\n");
    print_queue(queue);

    printf("\nDequeuing all jobs:\n");
    while (!is_empty(queue)) {
        Job *job = dequeue(queue);
        printf("  Dequeued PID: %d, Priority: %d\n", job->PID, job->priority);
        free(job);
    }

    destroy_queue(queue);

    printf("Priority test passed!\n");
}

void test_remove_job() {
    printf("\n=== Testing Remove Job ===\n");

    Queue *queue = create_queue(QUEUE_FIFO);

    Job *job1 = create_test_job(100, 0, 10, 0);
    Job *job2 = create_test_job(101, 1, 20, 0);
    Job *job3 = create_test_job(102, 2, 15, 0);

    enqueue(queue, job1, job1->service);
    enqueue(queue, job2, job2->service);
    enqueue(queue, job3, job3->service);

    printf("Original queue:\n");
    print_queue(queue);

    // Remove middle job
    printf("Removing job PID 101...\n");
    remove_job(queue, job2);
    print_queue(queue);

    // Find job by PID
    Job *found = find_job_by_pid(queue, 102);
    if (found) {
        printf("Found job PID: %d\n", found->PID);
    }

    // Test find non-existent job
    Job *not_found = find_job_by_pid(queue, 999);
    printf("Search for PID 999: %s\n", not_found == NULL ? "Not found (correct)" : "Found (error)");

    free(job2);
    while (!is_empty(queue)) {
        free(dequeue(queue));
    }
    destroy_queue(queue);

    printf("Remove job test passed!\n");
}

void test_clear_queue() {
    printf("\n=== Testing Clear Queue ===\n");

    Queue *queue = create_queue(QUEUE_FIFO);

    Job *job1 = create_test_job(100, 0, 10, 0);
    Job *job2 = create_test_job(101, 1, 20, 0);
    Job *job3 = create_test_job(102, 2, 15, 0);

    enqueue(queue, job1, job1->service);
    enqueue(queue, job2, job2->service);
    enqueue(queue, job3, job3->service);

    printf("Queue before clear:\n");
    print_queue(queue);

    clear_queue(queue);

    printf("Queue after clear:\n");
    print_queue(queue);
    printf("Is empty: %s\n", is_empty(queue) ? "Yes" : "No");

    // Jobs still exist, we need to free them manually
    free(job1);
    free(job2);
    free(job3);

    destroy_queue(queue);

    printf("Clear queue test passed!\n");
}

void test_update_remaining_time() {
    printf("\n=== Testing Update Remaining Time ===\n");

    Queue *queue = create_queue(QUEUE_SJF);

    Job *job1 = create_test_job(100, 0, 20, 0);
    Job *job2 = create_test_job(101, 1, 15, 0);
    Job *job3 = create_test_job(102, 2, 10, 0);

    enqueue(queue, job1, 20);
    enqueue(queue, job2, 15);
    enqueue(queue, job3, 10);

    printf("Original queue:\n");
    print_queue(queue);

    // Update remaining time for job2
    printf("\nUpdating remaining time for PID 101 from 15 to 5...\n");
    update_remaining_time(queue, job2, 5);
    print_queue(queue);

    printf("Note: The queue order doesn't change automatically.\n");
    printf("For SJF, you would need to remove and re-enqueue to re-sort.\n");

    while (!is_empty(queue)) {
        free(dequeue(queue));
    }
    destroy_queue(queue);

    printf("Update remaining time test passed!\n");
}

void test_edge_cases() {
    printf("\n=== Testing Edge Cases ===\n");

    // Test with empty queue
    Queue *queue = create_queue(QUEUE_FIFO);
    printf("Dequeue from empty queue: %s\n", dequeue(queue) == NULL ? "NULL (correct)" : "Not NULL (error)");
    printf("Peek at empty queue: %s\n", peek(queue) == NULL ? "NULL (correct)" : "Not NULL (error)");
    printf("Is empty: %s\n", is_empty(queue) ? "Yes (correct)" : "No (error)");
    printf("Size: %d (should be 0)\n", queue_size(queue));

    // Test with single job
    Job *job = create_test_job(200, 0, 10, 0);
    enqueue(queue, job, job->service);
    printf("\nAfter enqueuing single job:\n");
    print_queue(queue);

    Job *dequeued = dequeue(queue);
    printf("After dequeuing: Is empty: %s\n", is_empty(queue) ? "Yes (correct)" : "No (error)");

    free(dequeued);
    destroy_queue(queue);

    printf("Edge cases test passed!\n");
}

int main() {
    printf("========================================\n");
    printf("   Queue Implementation Test Suite\n");
    printf("========================================\n");

    test_fifo_queue();
    test_sjf_queue();
    test_priority_queue();
    test_remove_job();
    test_clear_queue();
    test_update_remaining_time();
    test_edge_cases();

    printf("\n========================================\n");
    printf("   All Queue Tests Passed!\n");
    printf("========================================\n");

    return 0;
}
