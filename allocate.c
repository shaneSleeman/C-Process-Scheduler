#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#define PROCESS_NAME_LENGTH 8

// Todo: split functions, make sure other marks
// clean files pushed

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
    int quantumTime = 0;
    int totalTurnaround = 0;
    int previousTurnaround = 0;

    // Executed processes array
    int executed[processCount];
    for (int i = 0; i < processCount; i++) {
        executed[i] = 0;
    }

    int remain = processCount;
    
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

        printf("%d,FINISHED,process_name=%s,proc_remaining=%d\n", 
                totalTime, processes[shortest].name, remain);
    }

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
    int lastExecuted = -1;

    // Executed processes array and their remaining times
    int executed[processCount];
    int remainingTime[processCount];
    for (int i = 0; i < processCount; i++) {
        executed[i] = 0;
        remainingTime[i] = processes[i].time;
    }

    int remain = processCount;

    while (remain > 0) {
        
    }

    //printf("Makespan %d\n", totalTime);
}
