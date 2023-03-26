#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PROCESS_NAME_LENGTH 8

// Process data structure
typedef struct {
    int arrival;
    char name[PROCESS_NAME_LENGTH];
    int time;
    int memory;
} Process;

void shortestJobFirst(Process processes[], 
        int processCount, int memory, int quantum);

// Trying to pass: ./allocate -f process.txt -s SJF -m infinite -q 3

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

    return 0;
}

void shortestJobFirst(Process processes[], int processCount, int memory, int quantum) {

    // Array of process indexes, ordered by process time
    int shortestOrder[10000]; // Some const for max processes

    for (int i = 0; i < processCount; i++) {
        shortestOrder[i] = i;
    }

    for (int i = 0; i < processCount; i++) {
        for (int j = i + 1; j < processCount; j++) {
            if (processes[shortestOrder[i]].time > processes[shortestOrder[j]].time) {
                int c = shortestOrder[i];
                shortestOrder[i] = shortestOrder[j];
                shortestOrder[j] = c;
            }
        }
    }
}
