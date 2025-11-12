# Scheduling
CS5600 Course Project3. Design and Develop a scheduler for a basic operating system.

```C
/////////// queue.h DONE

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



///////////////// job.h DONE

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

void wait(Job *job);
void run(Job *job);
void sleep(Job *job);




//////////////// utils DONE

void os_srand(int seed);
void os_rand();

int IO_request();
int IO_complete();




//////////////// clock.h

void init_clock();
int current_clock();
void next_tick();




///////////// scheduler.h

struct Global_Info {
    int total_simulate_time;
    int total_number_of_job;
    int shortest_job_completion_time;
    int Longest_job_completion_time;
    int average_completion;
    int average_ready;
};

struct {
    Queue jobs;
};

void psjf();
void robin();
void mlfs();

// 

// main.c

```

## How to run

``` txt
First, use "make" or "make scheduling".

Usage:  %s [--policy sjf|rr|mlfq] [--quantum N]
        Reads job definitions from stdin in the format PID:Arrival:Service:Priority.\n
        Examples:
              %s --policy sjf < input.txt
              %s --policy rr --quantum 4 < input.txt

             
For seperate test, Usage:
P-sjf: 
make run-sjf INPUT=path/to/input_file.txt

RR:
make run-rr INPUT=path/to/input_file.txt QUANTUM=N

MLFQ:
make run-mlfq INPUT=path/to/input_file.txt
```