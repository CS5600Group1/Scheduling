#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "include/job.h"
#include "include/scheduler.h"

static void trim_trailing(char *s) {
    if (s == NULL) return;
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[--len] = '\0';
    }
}

static void print_usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s [--policy sjf|rr|mlfq] [--quantum N]\n"
            "Reads job definitions from stdin in the format PID:Arrival:Service:Priority.\n"
            "Examples:\n"
            "  %s --policy sjf < input.txt\n"
            "  %s --policy rr --quantum 4 < input.txt\n",
            prog,
            prog,
            prog);
}

static int parse_policy(const char *arg, SchedulerPolicy *out_policy) {
    if (arg == NULL || out_policy == NULL) {
        return -1;
    }

    if (strcmp(arg, "sjf") == 0) {
        *out_policy = SCHED_POLICY_SJF;
        return 0;
    }
    if (strcmp(arg, "rr") == 0) {
        *out_policy = SCHED_POLICY_RR;
        return 0;
    }
    if (strcmp(arg, "mlfq") == 0) {
        *out_policy = SCHED_POLICY_MLFQ;
        return 0;
    }
    return -1;
}

int main(int argc, char *argv[]){
    os_srand(1);
    
    const char *policy_arg = "sjf";
    int time_quantum = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--policy") == 0 || strcmp(argv[i], "-p") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --policy requires an argument\n");
                print_usage(argv[0]);
                return 1;
            }
            policy_arg = argv[++i];
        } else if (strcmp(argv[i], "--quantum") == 0 || strcmp(argv[i], "-q") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --quantum requires an argument\n");
                print_usage(argv[0]);
                return 1;
            }
            time_quantum = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Error: unknown argument '%s'\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    SchedulerPolicy policy;
    if (parse_policy(policy_arg, &policy) != 0) {
        fprintf(stderr, "Error: unknown policy '%s'. Expected sjf, rr, or mlfq.\n", policy_arg);
        print_usage(argv[0]);
        return 1;
    }

    if (policy == SCHED_POLICY_RR && time_quantum <= 0) {
        fprintf(stderr, "Error: round-robin policy requires a positive --quantum value.\n");
        return 1;
    }

    char line[256];

    Job **jobs = NULL;
    size_t count = 0;
    size_t capacity = 0;

    while (fgets(line, sizeof(line), stdin) != NULL) {
        trim_trailing(line);

        // skip empty or comment lines
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0' || *p == '#') continue;

        int pid, arrival, service, priority;
        if (sscanf(p, "%d:%d:%d:%d", &pid, &arrival, &service, &priority) != 4) {
            fprintf(stderr, "Invalid line: %s\n", line);
            return 1;
        }
        if (arrival < 0 || service <= 0) {
            fprintf(stderr, "Invalid values (arrival>=0, service>0): %s\n", line);
            return 1;
        }

        if (count == capacity) {
            size_t newcap = capacity == 0 ? 4 : capacity * 2;
            Job **tmp = (Job**)realloc(jobs, newcap * sizeof(Job*));
            if (!tmp) {
                fprintf(stderr, "Out of memory\n");
                return 1;
            }
            jobs = tmp;
            capacity = newcap;
        }

        init_Job(&jobs[count], pid, arrival, service, priority);
        count++;
    }

    if (count == 0) {
        fprintf(stderr, "No jobs found in input.\n");
        free(jobs);
        return 1;
    }

    printf("Loaded %zu job(s). Dispatching policy: %s\n", count, policy_arg);
    schedule(jobs, (int)count, policy, time_quantum);

    for (size_t i = 0; i < count; ++i) {
        free(jobs[i]);
    }

    free(jobs);

    return 0;
}