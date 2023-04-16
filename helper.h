#ifndef HELPER_H
#define HELPER_H
#include "process.h"

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>

#define MEMORY_CAPACITY 2048
#define MAX_PROCESSES INT16_MAX // Largest int that doesn't cause error
#define NUM_ARGS 4
#define MIN_QUANTUM 1
#define MAX_QUANTUM 3
#define EMPTY -1
#define PRECISION 100

typedef struct {
    char *file;
    bool schedule;
    bool memoryChoice;
    int quantum;
} Arguments;

int shortestProcess(Process processes[], int processCount, int totalTime, bool executed[]);
void printPerformance(int turnaround, double maxOverhead, double totalOverhead, int processCount);
void updatePerformance(Process processes[], int totalTime, int i, int *turnaround, double *maxOverhead, double *totalOverhead);
int lowerTime(int totalTime, bool executed[], Process processes[], int processCount, int quantum);
void modifyMemory(int memory[], int i, int start, int length, int fill);
int lowestMultiple(int n, int i);
int nextFree(int memory[], Process processes[], int processCount, int length);
void readyProcess(int processCount, int totalTime, int quantum, int memory[], Process processes[], bool useSJF, bool offset, int *readyTime);
int compareProcess(const void *a, const void *b);
bool parseArguments(int argc, char **argv, Arguments *args);

#endif /* HELPER_H */