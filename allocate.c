#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

// Defined helper functions
#include "helper.h"
#include "process.h"

// Todo: split functions, make sure other marks
// clean files pushed, remove folder thing
// find more tests

void shortestJobFirst(Process processes[], 
        int processCount, int memoryChoice, int quantum);
void roundRobin(Process processes[], int processCount, 
        int memoryChoice, int quantum);
int shortestProcess(Process processes[], int processCount, int totalTime, int executed[]);
void printPerformance(int turnaround, double maxOverhead, double totalOverhead, int processCount);
void updatePerformance(Process processes[], int totalTime, int i, int *turnaround, double *maxOverhead, double *totalOverhead);
int lowerTime(int totalTime, int executed[], Process processes[], int processCount, int quantum);
void modifyMemory(int memory[], int i, int start, int length, int fill);
int lowestMultiple(int n, int i);
int nextFree(int memory[], Process processes[], int processCount, int length);

int main(int argc, char **argv) {

    // Storing arguments
    char *file = NULL;
    int schedule = 0;
    int memoryChoice = 0;
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
                memoryChoice = 1;
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
    p.started = 0;
    p.memoryStart = 0;
    while (fscanf(processesFile, "%d %s %d %d", 
            &p.arrival, p.name, &p.time, &p.memory) == 4) {
        processes[processesCount++] = p;
    }

    fclose(processesFile);

    if(schedule == 0) {
        shortestJobFirst(processes, processesCount, memoryChoice, quantum);
    }
    else if(schedule == 1) {
        roundRobin(processes, processesCount, memoryChoice, quantum);
    }

    return 0;
}

// Todo: bunch of edge cases handling
void shortestJobFirst(Process processes[], int processCount, int memoryChoice, int quantum) {
    
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

    // Memory
    int memory[MEMORY_CAPACITY];
    for(int i = 0; i < MEMORY_CAPACITY; i++) {
        memory[i] = -1;
    }

    while (remain > 0) {

        int shortest = shortestProcess(processes, processCount, totalTime, executed);

        // If none available to execute
        if (shortest == -1) {
            totalTime++;
            continue;
        }

        // Print when processes are ready
        if(memoryChoice) {
            for(int i = 0; i < processCount; i++) {
                if(totalTime >= lowestMultiple(
                            processes[i].arrival, quantum) &&
                            processes[i].started == 0) {
                    processes[i].memoryStart = nextFree(memory, processes, processCount, processes[i].memory);
                    modifyMemory(memory, i, processes[i].memoryStart, processes[i].memory, 1);

                    printf("%d,READY,process_name=%s,assigned_at=%d\n", 
                            lowestMultiple(processes[i].arrival, quantum),
                            processes[i].name, processes[i].memoryStart);
                    processes[i].started = 1;
                    //currentMemory += processes[i].memory;
                }
            }
        }

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

        if(memoryChoice) {
            for(int i = 0; i < processCount; i++) {
                if(totalTime - quantum >= lowestMultiple(
                            processes[i].arrival, quantum) &&
                            processes[i].started == 0) {
                    processes[i].memoryStart = nextFree(memory, processes, processCount, processes[i].memory);
                    modifyMemory(memory, i, processes[i].memoryStart, processes[i].memory, 1);
                    printf("%d,READY,process_name=%s,assigned_at=%d\n", 
                            lowestMultiple(processes[i].arrival, quantum),
                            processes[i].name, processes[i].memoryStart);
                    processes[i].started = 1;
                }
            }
        }
        printf("%d,FINISHED,process_name=%s,proc_remaining=%d\n", 
                totalTime, processes[shortest].name, lowerTime(totalTime, executed, processes, processCount, quantum));
        
        // Designate that the process is complete, for memory reassignment
        modifyMemory(memory, shortest, processes[shortest].memoryStart, processes[shortest].memory, 0);
    }

    printPerformance(turnaround, maxOverhead, totalOverhead, processCount);
    printf("Makespan %d\n", totalTime);
}

// PROBLEM: best-fit introduces ready delay...
// may also be problem with sjf
void roundRobin(Process processes[], int processCount, int memoryChoice, int quantum) {

    int totalTime = 0;
    int lastExecuted = -1; // Last process, avoid reprint

    // Executed processes array and their remaining times
    int executed[processCount];
    int remainingTime[processCount];
    int prevRemainingTime[processCount];
    for (int i = 0; i < processCount; i++) {
        executed[i] = 0;
        remainingTime[i] = processes[i].time;
        prevRemainingTime[i] = -1;
    }

    int remain = processCount;

    // Hold performance stats
    int turnaround = 0;
    double maxOverhead = 0;
    double totalOverhead = 0;

    // Memory
    int memory[MEMORY_CAPACITY];
    for(int i = 0; i < MEMORY_CAPACITY; i++) {
        memory[i] = -1;
    }

    // Aiding bug fix
    int prevProcess = 0;

    while (remain > 0) {

        // Avoids accidental quantum skips
        int readyTime = -1;

        // Print when processes are ready
        int previousRunning = -1;
        for (int i = 0; i < processCount; i++) {

            // Avoid incorrect run order
            int printedReady = 0;

            if(memoryChoice) {
                for(int i = 0; i < processCount; i++) {
                    if(totalTime >= lowestMultiple(
                                processes[i].arrival, quantum) &&
                                processes[i].started == 0) {
                        if(nextFree(memory, processes, processCount, processes[i].memory) != -1) {
                            processes[i].memoryStart = nextFree(memory, processes, processCount, processes[i].memory);
                            modifyMemory(memory, i, processes[i].memoryStart, processes[i].memory, 1);
                            readyTime = totalTime;

                            printf("%d,READY,process_name=%s,assigned_at=%d\n", 
                                    lowestMultiple(totalTime, quantum),
                                    processes[i].name, processes[i].memoryStart);
                            processes[i].started = 1;
                            printedReady++;
                        }
                    }
                }
            }

            //if(printedReady) break;

            // For best-fit, only start process if it's started
            int startedCheck = 1;
            if(memoryChoice == 1) {
                startedCheck = processes[i].started == 1;
            }

            // If appropriate arrival and not executed yet
            if (executed[i] == 0 && processes[i].arrival <= totalTime && startedCheck) {
                totalTime += quantum;

                // Fix quantum skip bug
                if(readyTime != -1 && readyTime != totalTime - quantum) {
                    totalTime = readyTime + quantum;
                    remainingTime[prevProcess] += quantum;
                }

                // Only print first running instance
                if (i != lastExecuted) {

                    // Avoid time lag
                    if(previousRunning == totalTime - quantum) {
                        totalTime += quantum;
                        remainingTime[prevProcess] -= quantum;
                    }
                    if(memoryChoice) {
                        if(totalTime - quantum < previousRunning) {
                            totalTime = quantum + quantum + previousRunning;
                        }
                    }
                    if(prevRemainingTime[i] == remainingTime[i]) {
                        remainingTime[i] -= quantum;
                    }
                    printf("%d,RUNNING,process_name=%s,remaining_time=%d\n", 
                            totalTime - quantum, processes[i].name, remainingTime[i]);
                    prevProcess = i;
                    lastExecuted = i;
                    previousRunning = totalTime - quantum;
                    prevRemainingTime[i] = remainingTime[i];
                }

                // Finish process when no more remaining time
                remainingTime[i] -= quantum;
                if (remainingTime[i] <= 0) {
                    executed[i] = 1;
                    remain--;

                    updatePerformance(processes, totalTime, i, &turnaround, 
                            &maxOverhead, &totalOverhead);

                    if(memoryChoice) {
                        for(int i = 0; i < processCount; i++) {
                            if(totalTime - quantum >= lowestMultiple(
                                        processes[i].arrival, quantum) &&
                                        processes[i].started == 0) {
                                if(nextFree(memory, processes, processCount, processes[i].memory) != -1) {
                                    processes[i].memoryStart = nextFree(memory, processes, processCount, processes[i].memory);
                                    modifyMemory(memory, i, processes[i].memoryStart, processes[i].memory, 1);
                                    printf("%d,READY,process_name=%s,assigned_at=%d\n", 
                                            lowestMultiple(totalTime, quantum),
                                            processes[i].name, processes[i].memoryStart);
                                    processes[i].started = 1;
                                }
                            }
                        }
                    }
                    printf("%d,FINISHED,process_name=%s,proc_remaining=%d\n", 
                            totalTime, processes[i].name, remain);

                    // Designate that the process is complete, for memory reassignment
                    modifyMemory(memory, i, processes[i].memoryStart, processes[i].memory, 0);
                    
                    // End current iteration to avoid incorrect order
                    if(memoryChoice) break;
                }
            }
        }
    }

    printPerformance(turnaround, maxOverhead, totalOverhead, processCount);
    printf("Makespan %d\n", totalTime);
}