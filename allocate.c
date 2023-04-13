#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Defined helper function
#include "helper.h"
#include "process.h"

// Todo:
// split functions, make sure other marks
// include redundancies and comments
// clean notation and var names indentations
// attempt task 4
// line widths
// reduce excess commits
// memory leaks?

void scheduler(Process processes[], int processCount,
  int scheduleChoice, int memoryChoice, int quantum);

int main(int argc, char **argv) {

  // Storing arguments
  char * file = NULL;
  int scheduleChoice = 0, memoryChoice = 0, quantum = 0, processesCount = 0;

  // Retrieve arguments
   for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-f")) {
        if (i + 1 < argc) file = argv[++i];
        else return 1;
    } else if (!strcmp(argv[i], "-s")) {
        if (i + 1 < argc && !strcmp(argv[i + 1], "RR")) {
            scheduleChoice = 0;
            i++;
        } 
        else if (i + 1 < argc && !strcmp(argv[i + 1], "SJF")) i++;
        else return 1;
    } else if (!strcmp(argv[i], "-m")) {
        if (i + 1 < argc && !strcmp(argv[i + 1], "best-fit")) {
            memoryChoice = 1;
            i++;
        }
        else if (i + 1 < argc && !strcmp(argv[i + 1], "infinite")) i++;
        else return 1;
    } else if (!strcmp(argv[i], "-q")) {
        if (i + 1 < argc && atoi(argv[i + 1]) >= 1 && atoi(argv[i + 1]) <= 3) quantum = atoi(argv[++i]);
        else return 1;
    }
  }

  FILE * processesFile = fopen(file, "r");

  if (processesFile == NULL) return 1;

  Process processes[MAX_PROCESSES], p;

  p.memoryStart = -1;
  while (fscanf(processesFile, "%d %s %d %d", &
      p.arrival, p.name, & p.time, & p.memory) == 4) processes[processesCount++] = p;

  fclose(processesFile);

  qsort(processes, processesCount, sizeof(Process), compareProcess);

  scheduler(processes, processesCount, scheduleChoice, memoryChoice, quantum);

  return 0;
}

void scheduler(Process processes[], int processCount, int scheduleChoice, int memoryChoice, int quantum) {

  int totalTime = 0, lastExecuted = -1, turnaround = 0, prevProcess = 0, memory[MEMORY_CAPACITY], remain = processCount; // Last process, avoid reprint

  // Memory
  for (int i = 0; i < MEMORY_CAPACITY; i++) memory[i] = -1;

  // Executed processes array and their remaining times
  int executed[processCount], remainingTime[processCount], prevRemainingTime[processCount];
  for (int i = 0; i < processCount; i++) {
    executed[i] = 0;
    remainingTime[i] = processes[i].time;
    prevRemainingTime[i] = -1;
  }

  // Hold performance stats
  double maxOverhead = 0.0, totalOverhead = 0.0;

  while (remain > 0) {

    // Avoids accidental quantum skips
    int readyTime = -1, previousRunning = -1;

    for (int i = 0; i < processCount; i++) {

      // Print when processes are ready
      if(memoryChoice) readyProcess(processCount, totalTime, quantum, memory, processes, scheduleChoice, 0, &readyTime);

      if (!scheduleChoice) {
        int shortest = shortestProcess(processes, processCount, totalTime, executed);

        // If none available to execute
        if (shortest == -1) {
            totalTime++; 
            continue;
        }

        printf("%d,RUNNING,process_name=%s,remaining_time=%d\n",
          totalTime, processes[shortest].name, processes[shortest].time);

        // Add quantums passeds to total
        // Could check for completion each quantum,
        // but doesn't seem necessary at the moment
        // Signifying as executed
        int quantums = 0;
        while (quantums < processes[shortest].time) quantums += quantum;
        totalTime += quantums;
        executed[shortest] = 1;

        remain--;

        updatePerformance(processes, totalTime, shortest, &turnaround, &
          maxOverhead, & totalOverhead);

        if (memoryChoice) {
          readyProcess(processCount, totalTime, quantum, memory, processes, scheduleChoice, 1, &readyTime);
          modifyMemory(memory, shortest, processes[shortest].memoryStart, processes[shortest].memory, 0);
        }

        printf("%d,FINISHED,process_name=%s,proc_remaining=%d\n",
          totalTime, processes[shortest].name, lowerTime(totalTime, executed, processes, processCount, quantum));
      } else {
        // For best-fit, only start process if it's started
        int startedCheck = 1;
        if (memoryChoice == 1) startedCheck = processes[i].memoryStart != -1;

        // If appropriate arrival and not executed yet
        if (executed[i] == 0 && processes[i].arrival <= totalTime && startedCheck) {
          totalTime += quantum;

          // Fix quantum skip bug
          if (readyTime != -1 && readyTime != totalTime - quantum) {
            totalTime = readyTime + quantum;
            remainingTime[prevProcess] += quantum;
          }

          // Only print first running instance
          if (i != lastExecuted) {

            // Avoid time lag
            if (previousRunning == totalTime - quantum) {
              totalTime += quantum;
              remainingTime[prevProcess] -= quantum;
            }
            if (memoryChoice) {
              if (totalTime - quantum < previousRunning) totalTime = quantum + quantum + previousRunning;
            }
            if (prevRemainingTime[i] == remainingTime[i]) remainingTime[i] -= quantum;

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

            updatePerformance(processes, totalTime, i, & turnaround, &
              maxOverhead, & totalOverhead);

            printf("%d,FINISHED,process_name=%s,proc_remaining=%d\n",
              totalTime, processes[i].name, remain);

            // End current iteration to avoid incorrect order
            if (memoryChoice) {
                modifyMemory(memory, i, processes[i].memoryStart, processes[i].memory, 0); 
                break;
            }
          }
        }
      }
    }
  }

  printPerformance(turnaround, maxOverhead, totalOverhead, processCount);
  printf("Makespan %d\n", totalTime);
}