#ifndef PROCESS_H
#define PROCESS_H

#define PROCESS_NAME_LENGTH 8

typedef struct {
    int arrival;
    char name[PROCESS_NAME_LENGTH];
    int time;
    int memory;
    int started;
    int memoryStart;
} Process;

#endif /* PROCESS_H */
