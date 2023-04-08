#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#define PROCESS_NAME_LENGTH 8

// Todo: split functions, make sure other marks
// clean files pushed, remove folder thing
// find more tests

// Process data structure
typedef struct {
    int arrival;
    char name[PROCESS_NAME_LENGTH];
    int time;
    int memory;
} Process;

void shortestJobFirst(Process processes[], 
        int processCount, int memory, int quantum);
int shortestProcess(Process processes[], int processCount, int totalTime, int executed[]);
void roundRobin(Process processes[], int processCount, 
        int memory, int quantum);
void printPerformance(int turnaround, double maxOverhead, double totalOverhead, int processCount);
void updatePerformance(Process processes[], int totalTime, int i, int *turnaround, double *maxOverhead, double *totalOverhead);

int main(int argc, char **argv) {

    // Storing arguments
    char *file = NULL;
    int schedule = 0;
    int memory = 0;
    int quantum = 0;

    // Retrieve and store arguments
    // NEED TO ERRORS??
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-f")) {
            if (i + 1 < argc) {
                file = argv[++i];
            }
        } 
        else if (!strcmp(argv[i], "-s")) {
            if (i + 1 < argc && !strcmp(argv[i + 1], "RR")) {
                schedule = 1;
                i++;
            }
        } 
        else if (!strcmp(argv[i], "-m")) {
            if (i + 1 < argc && !strcmp(argv[i + 1], "best-fit")) {
                memory = 1;
                i++;
            }
        } 
        else if (!strcmp(argv[i], "-q")) {
            if (i + 1 < argc) {
                quantum = atoi(argv[++i]);
            }
        }
    }

    FILE *processesFile = fopen(file, "r");

    if (processesFile == NULL) {
        printf("Failed to open file.");
        return 1;
    }

    Process processes[10000]; // Figure out const or max
    int processesCount = 0;

    Process p;
    while (fscanf(processesFile, "%d %s %d %d", 
            &p.arrival, p.name, &p.time, &p.memory) == 4) {
        processes[processesCount++] = p;
    }

    fclose(processesFile);

    if(schedule == 0) {
        shortestJobFirst(processes, processesCount, memory, quantum);
    }
    else if(schedule == 1) {
        roundRobin(processes, processesCount, memory, quantum);
    }

    return 0;
}

// Todo: bunch of edge cases handling
void shortestJobFirst(Process processes[], int processCount, int memory, int quantum) {
    
    int totalTime = 0;

    // Executed processes array
    int executed[processCount];
    for (int i = 0; i < processCount; i++) {
        executed[i] = 0;
    }

    int remain = processCount;

    // Hold performance stats
    int turnaround = 0;
    double maxOverhead = 0.0;
    double totalOverhead = 0.0;
    
    while (remain > 0) {
        int shortest = shortestProcess(processes, processCount, totalTime, executed);

        printf("%d,RUNNING,process_name=%s,remaining_time=%d\n", 
                totalTime, processes[shortest].name, processes[shortest].time);

        // Add quantums passeds to total
        // Could check for completion each quantum,
        // but doesn't seem necessary at the moment
        // Signifying as executed
        int quantums = 0;
        while(quantums < processes[shortest].time) {
            quantums += quantum;
        }
        totalTime += quantums;
        executed[shortest] = 1;

        remain--;

        updatePerformance(processes, totalTime, shortest, &turnaround, 
                            &maxOverhead, &totalOverhead);

        printf("%d,FINISHED,process_name=%s,proc_remaining=%d\n", 
                totalTime, processes[shortest].name, remain);
    }

    printPerformance(turnaround, maxOverhead, totalOverhead, processCount);
    printf("Makespan %d\n", totalTime);
}

// Finds the shortest remaining process
int shortestProcess(Process processes[], int processCount, int totalTime, int executed[]) {
    int shortest = -1;
    int minimum = INT_MAX;

    // Find the index of the shortest non-executed process
    // Always begins with the first process
    for (int i = 0; i < processCount; i++) {
        if (executed[i] == 0 &&
                processes[i].time < minimum &&
                processes[i].arrival <= totalTime) {
            shortest = i;
            minimum = processes[i].time;
        }
    }

    return shortest;
}

void roundRobin(Process processes[], int processCount, int memory, int quantum) {

    int totalTime = 0;
    int lastExecuted = -1; // Last process, avoid reprint

    // Executed processes array and their remaining times
    int executed[processCount];
    int remainingTime[processCount];
    for (int i = 0; i < processCount; i++) {
        executed[i] = 0;
        remainingTime[i] = processes[i].time;
    }

    int remain = processCount;

    // Hold performance stats
    int turnaround = 0;
    double maxOverhead = 0;
    double totalOverhead = 0;

    while (remain > 0) {
        for (int i = 0; i < processCount; i++) {

            // If appropriate arrival and not executed yet
            if (executed[i] == 0 && processes[i].arrival <= totalTime) {
                totalTime += quantum;

                // Only print first running instance
                if (i != lastExecuted) {
                    printf("%d,RUNNING,process_name=%s,remaining_time=%d\n", 
                            totalTime - quantum, processes[i].name, remainingTime[i]);
                    lastExecuted = i;
                }

                // Finish process when no more remaining time
                remainingTime[i] -= quantum;
                if (remainingTime[i] <= 0) {
                    executed[i] = 1;
                    remain--;

                    updatePerformance(processes, totalTime, i, &turnaround, 
                            &maxOverhead, &totalOverhead);

                    printf("%d,FINISHED,process_name=%s,proc_remaining=%d\n", 
                            totalTime, processes[i].name, remain);
                }
            }
        }
    }

    printPerformance(turnaround, maxOverhead, totalOverhead, processCount);
    printf("Makespan %d\n", totalTime);
}

void updatePerformance(Process processes[], int totalTime, int process, int *turnaround, 
        double *maxOverhead, double *totalOverhead) {
    
    // Current process' turnaround and time overhead
    int processTurnaround = totalTime - processes[process].arrival;

    // Update turnaround
    *turnaround += processTurnaround;

    // Current process' overhead
    double processOverhead = processTurnaround / (double)processes[process].time;

    // Track max overhead and update total overhead
    if (processOverhead > *maxOverhead) {
        *maxOverhead = processOverhead;
    }
    *totalOverhead += processOverhead;
}

void printPerformance(int turnaround, double maxOverhead, double totalOverhead, int processCount) {
    printf("Turnaround time %d\n", (int)ceil(turnaround / (double)processCount));
    printf("Time overhead %.2f %.2f\n", maxOverhead, totalOverhead / processCount);
}