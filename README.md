# Scheduling
CS5600 Course Project3. Design and Develop a scheduler for a basic operating system.

```C
//queue.h DONE

struct Queue{
    Node *head;
    Node *tail;
};

struct Node{
    Job *value;
    Node *next;
};

void init_queue(Queue*)
void enqueue(Job*)
Job dequeue()

bool empty(Queue*)
bool getHead(Queue*)
bool getTail(Queue*)

//job.h DONE

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

//utils DONE

void os_srand(int seed);
void os_rand();

int IO_request();
int IO_complete();

// clock.h

void init_clock();
int current_clock();
void next_tick();

// sceduler.h

struct Global_Info {
    int total_simulate_time;
    int total_number_of_job;
    int shortest_job_completion_time;
    int Longest_job_completion_time;
    int average_completion;
    int average_ready;
};

struct Device{
    Queue jobs;
};

void psjf();
void robin();
void mlfs();


// main.c

```