#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#define PROCESS_NAME_LENGTH 8
#define MEMORY_CAPACITY 2048

// Todo: split functions, make sure other marks
// clean files pushed, remove folder thing
// find more tests

// Process data structure
typedef struct {
    int arrival;
    char name[PROCESS_NAME_LENGTH];
    int time;
    int memory;
    int started;
    int memoryStart;
} Process;

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

// Assigning and clearing memory functions
// will combine
void fillMemory(int memory[], int i, int start, int length) {
    for(int j = start; j < start + length; j++) {
        memory[j] = i;
    }
}

void clearMemory(int memory[], int i, int start, int length) {
    for(int j = start; j < start + length; j++) {
        memory[j] = -1;
    }
}

void shortestJobFirst(Process processes[], 
        int processCount, int memoryChoice, int quantum);
int shortestProcess(Process processes[], int processCount, int totalTime, int executed[]);
void roundRobin(Process processes[], int processCount, 
        int memoryChoice, int quantum);
void printPerformance(int turnaround, double maxOverhead, double totalOverhead, int processCount);
void updatePerformance(Process processes[], int totalTime, int i, int *turnaround, double *maxOverhead, double *totalOverhead);
int lowerTime(int totalTime, int executed[], Process processes[], int processCount, int quantum);

// Simplifies use of quantum time by checking
// arrival times are multiples of quantum
int lowestMultiple(int n, int i) {
    int result = (n / i) * i;
    if (result < n) {
        result += i;
    }

    return result;
}

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
                    fillMemory(memory, i, processes[i].memoryStart, processes[i].memory);

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
                    fillMemory(memory, i, processes[i].memoryStart, processes[i].memory);
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
        clearMemory(memory, shortest, processes[shortest].memoryStart, processes[shortest].memory);
    }

    printPerformance(turnaround, maxOverhead, totalOverhead, processCount);
    printf("Makespan %d\n", totalTime);
}

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

// PROBLEM: best-fit introduces ready delay...
// may also be problem with sjf
void roundRobin(Process processes[], int processCount, int memoryChoice, int quantum) {

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

            if(memoryChoice) {
                for(int i = 0; i < processCount; i++) {
                    if(totalTime >= lowestMultiple(
                                processes[i].arrival, quantum) &&
                                processes[i].started == 0) {
                        if(nextFree(memory, processes, processCount, processes[i].memory) != -1) {
                            processes[i].memoryStart = nextFree(memory, processes, processCount, processes[i].memory);
                            fillMemory(memory, i, processes[i].memoryStart, processes[i].memory);
                            readyTime = totalTime;

                            printf("%d,READY,process_name=%s,assigned_at=%d\n", 
                                    lowestMultiple(totalTime, quantum),
                                    processes[i].name, processes[i].memoryStart);
                            processes[i].started = 1;
                        }
                    }
                }
            }

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
                    printf("%d,RUNNING,process_name=%s,remaining_time=%d\n", 
                            totalTime - quantum, processes[i].name, remainingTime[i]);
                    prevProcess = i;
                    lastExecuted = i;
                    previousRunning = totalTime - quantum;
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
                                    fillMemory(memory, i, processes[i].memoryStart, processes[i].memory);
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
                    clearMemory(memory, i, processes[i].memoryStart, processes[i].memory);
                }
            }
            
            //if(ran) totalTime += quantum;
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
    printf("Time overhead %.2f %.2f\n", round(maxOverhead * 100) / 100, round(totalOverhead / processCount * 100) / 100);
}