#include "helper.h"
#include "process.h"

int lowerTime(int totalTime, bool executed[], Process processes[], int processCount, int quantum) {
    int n = 0, atLeast = totalTime - quantum;
    for(int i = 1; i < processCount; i++) {
        if(processes[i].arrival < atLeast &&
                executed[i] != true) n++;
    }
    return n;
}

// Finds the shortest remaining process
int shortestProcess(Process processes[], int processCount, int totalTime, bool executed[]) {
    int shortest = EMPTY, minimum = INT_MAX;

    // Find the index of the shortest non-executed process
    // Always begins with the first process
    for (int i = 0; i < processCount; i++) {
        if (executed[i] == false &&
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
    if (processOverhead > *maxOverhead) *maxOverhead = processOverhead;
    *totalOverhead += processOverhead;
}

void printPerformance(int turnaround, double maxOverhead, double totalOverhead, int processCount) {
    printf("Turnaround time %d\n", (int)ceil(turnaround / (double)processCount));
    printf("Time overhead %.2f %.2f\n", round(maxOverhead * PRECISION) / PRECISION, round(totalOverhead / processCount * PRECISION) / PRECISION);
}

/* Assign memory start+length to i, the index of a process
 * Or clearing memory
*/
void modifyMemory(int memory[], int i, int start, int length, int fill) {
    for(int j = start; j < start + length; j++) memory[j] = fill ? i : EMPTY;
}

// Simplifies use of quantum time by checking
// arrival times are multiples of quantum
int lowestMultiple(int n, int i) {
    int result = (n / i) * i;
    if (result < n) result += i;
    return result;
}

// Next free memory location for a given memory size
// Returns EMPTY if memory is full i.e. has no free spot
// Imperfect implementation, must fix for bestfit
int nextFree(int memory[], Process processes[], int processCount, int length) {
    int currentLocation = EMPTY, tally = 0, minGap = INT_MAX;

    for(int i = 0; i < MEMORY_CAPACITY; i++) {
        if(memory[i] != EMPTY) tally = 0;
        else {
            if(tally == 0) currentLocation = i;
            tally++;
        }
        
        if(tally >= length) {
            int gap = tally - length;

            if(gap < minGap) {
                minGap = gap;

                // If gap is perfect
                if(minGap == 0) return currentLocation;
            }
        }
    }

    // If no perfect fit, return best-fit
    if(minGap != INT_MAX) return currentLocation;

    // When there's no free location
    return EMPTY;
}

void readyProcess(int processCount, int totalTime, int quantum, int memory[], Process processes[], bool useSJF, bool offset, int *readyTime) {
    for (int i = 0; i < processCount; i++) {

        int rrCheck = useSJF ? true : (nextFree(memory, processes, processCount, processes[i].memory) != EMPTY),
          arrivalQuantum = lowestMultiple(processes[i].arrival, quantum);

        if (processes[i].memoryStart == EMPTY && ((offset && totalTime - quantum >= arrivalQuantum) || (!offset && totalTime >= arrivalQuantum && rrCheck))) {
            int freeMemoryIndex = nextFree(memory, processes, processCount, processes[i].memory);

            if (freeMemoryIndex != EMPTY) {
                processes[i].memoryStart = freeMemoryIndex;
                modifyMemory(memory, i, processes[i].memoryStart, processes[i].memory, 1);
                int printTime = offset ? arrivalQuantum : lowestMultiple(totalTime, quantum);

                printf("%d,READY,process_name=%s,assigned_at=%d\n", printTime, processes[i].name, processes[i].memoryStart);

                if (!offset) *readyTime = totalTime;
            }
        }
    }
}

int compareProcess(const void *a, const void *b) {
    const Process *processA = (const Process *)a, *processB = (const Process *)b;

    if (processA->arrival != processB->arrival) return processA->arrival - processB->arrival;

    if (processA->time != processB->time) return processA->time - processB->time;

    return strcmp(processA->name, processB->name);
}

bool parseArguments(int argc, char **argv, Arguments *args) {
    args->file = NULL;
    args->schedule = false;
    args->memoryChoice = false;
    args->quantum = MIN_QUANTUM;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-f")) {
            if (i + 1 < argc) args->file = argv[++i];
            else return false;
        } else if (!strcmp(argv[i], "-s")) {
            if (i + 1 < argc && !strcmp(argv[i + 1], "RR")) {
                args->schedule = true;
                i++;
            } 
            else if (i + 1 < argc && !strcmp(argv[i + 1], "SJF")) i++;
            else return false;
        } else if (!strcmp(argv[i], "-m")) {
            if (i + 1 < argc && !strcmp(argv[i + 1], "best-fit")) {
                args->memoryChoice = true;
                i++;
            } 
            else if (i + 1 < argc && !strcmp(argv[i + 1], "infinite")) i++;
            else return false;
        } else if (!strcmp(argv[i], "-q")) {
            if (i + 1 < argc && atoi(argv[i + 1]) >= MIN_QUANTUM && atoi(argv[i + 1]) <= MAX_QUANTUM) args->quantum = atoi(argv[++i]);
            else return false;
        }
    }

    return args->file != NULL;
}