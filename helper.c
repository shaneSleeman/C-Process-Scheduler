#include "helper.h"
#include "process.h"
#include <math.h>
#include <stdio.h>
#include <limits.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

int lowerTime(int totalTime, int executed[], Process processes[], int processCount, int quantum) {
    int n = 0;
    int atLeast = totalTime - quantum;
    for(int i = 1; i < processCount; i++) {
        if(processes[i].arrival < atLeast &&
                executed[i] != 1) n++;
    }
    return n;
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
    printf("Time overhead %.2f %.2f\n", round(maxOverhead * 100) / 100, round(totalOverhead / processCount * 100) / 100);
}

// Assigning and clearing memory functions
void modifyMemory(int memory[], int i, int start, int length, int fill) {
    for(int j = start; j < start + length; j++) {
        if(fill) memory[j] = i;
        else {
            memory[j] = -1;
        }
    }
}

// Simplifies use of quantum time by checking
// arrival times are multiples of quantum
int lowestMultiple(int n, int i) {
    int result = (n / i) * i;
    if (result < n) {
        result += i;
    }

    return result;
}

// Next free memory location for a given memory size
// Imperfect implementation, must fix for bestfit
int nextFree(int memory[], Process processes[], int processCount, int length) {
    int currentLocation = 0;
    int tally = 0;
    for(int i = 0; i < MEMORY_CAPACITY; i++) {
        if(memory[i] != -1) {
            tally = 0;
        }
        else {
            if(tally == 0) {
                currentLocation = i;
            }
            tally++;
        }
        if(tally == length) return currentLocation;
    }

    // Flag for when there's no free memory, 0 if break
    return -1;
}

void readyProcess(int processCount, int totalTime, int quantum, int memory[], Process processes[], int sjf, int offset, int *readyTime, int *printedReady) {
    for (int i = 0; i < processCount; i++) {

        int rrCheck = sjf ? 1 : (nextFree(memory, processes, processCount, processes[i].memory) != -1);
        int arrivalQuantum = lowestMultiple(processes[i].arrival, quantum);

        if (processes[i].started == 0 && ((offset && totalTime - quantum >= arrivalQuantum) || (!offset && totalTime >= arrivalQuantum && rrCheck))) {
            int freeMemoryIndex = nextFree(memory, processes, processCount, processes[i].memory);

            if (freeMemoryIndex != -1) {
                processes[i].memoryStart = freeMemoryIndex;
                modifyMemory(memory, i, processes[i].memoryStart, processes[i].memory, 1);
                int printTime = offset ? arrivalQuantum : lowestMultiple(totalTime, quantum);

                printf("%d,READY,process_name=%s,assigned_at=%d\n", printTime, processes[i].name, processes[i].memoryStart);
                processes[i].started = 1;

                if (!offset) {
                    *readyTime = totalTime;
                    printedReady = printedReady + 1;
                }
            }
        }
    }
}

void catch_error(jmp_buf buf) {
    longjmp(buf, 1);
}

void parseArguments(int argc, char **argv, char **file, int *schedule, int *memoryChoice, int *quantum, jmp_buf buf) {
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-f")) {
            if (i + 1 < argc) {
                *file = argv[++i];
            } else {
                catch_error(buf);
            }
        } else if (!strcmp(argv[i], "-s")) {
            if (i + 1 < argc && !strcmp(argv[i + 1], "RR")) {
                *schedule = 1;
                i++;
            } 
            else if (i + 1 < argc && !strcmp(argv[i + 1], "SJF")) {
                i++;
            }
            else {
                catch_error(buf);
            }
        } else if (!strcmp(argv[i], "-m")) {
            if (i + 1 < argc && !strcmp(argv[i + 1], "best-fit")) {
                *memoryChoice = 1;
                i++;
            }
            else if (i + 1 < argc && !strcmp(argv[i + 1], "infinite")) {
                i++;
            }
            else {
                catch_error(buf);
            }
        } else if (!strcmp(argv[i], "-q")) {
            if (i + 1 < argc && atoi(argv[i + 1]) >= 1 && atoi(argv[i + 1]) <= 3) {
                *quantum = atoi(argv[++i]);
            } else {
                catch_error(buf);
            }
        }
    }
}
