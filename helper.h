#ifndef HELPER_H
#define HELPER_H
#include "process.h"

#define MEMORY_CAPACITY 2048

int shortestProcess(Process processes[], int processCount, int totalTime, int executed[]);
void printPerformance(int turnaround, double maxOverhead, double totalOverhead, int processCount);
void updatePerformance(Process processes[], int totalTime, int i, int *turnaround, double *maxOverhead, double *totalOverhead);
int lowerTime(int totalTime, int executed[], Process processes[], int processCount, int quantum);
void modifyMemory(int memory[], int i, int start, int length, int fill);
int lowestMultiple(int n, int i);
int nextFree(int memory[], Process processes[], int processCount, int length);

#endif /* HELPER_H */