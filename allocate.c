// Defined helper function
#include "helper.h"
#include "process.h"

void scheduler(Process processes[], int processCount,
  int memoryChoice, int quantum, bool sjf);

int main(int argc, char **argv) {

  Arguments args;
  if (!parseArguments(argc, argv, &args)) return 1;

  FILE * processesFile = fopen(args.file, "r");
  if (processesFile == NULL) return 1;

  Process processes[MAX_PROCESSES];
  int processesCount = 0;

  Process p;
  p.memoryStart = EMPTY;
  while (fscanf(processesFile, "%d %s %d %d", &
      p.arrival, p.name, & p.time, & p.memory) == NUM_ARGS) processes[processesCount++] = p;

  fclose(processesFile);

  qsort(processes, processesCount, sizeof(Process), compareProcess);

  scheduler(processes, processesCount, args.memoryChoice, args.quantum, !args.schedule);

  return 0;
}

void scheduler(Process processes[], int processCount, int memoryChoice, int quantum, bool sjf) {

  int totalTime = 0, lastExecuted = EMPTY, turnaround = 0, prevProcess = 0, memory[MEMORY_CAPACITY], remain = processCount; // Last process, avoid reprint

  // Memory
  for (int i = 0; i < MEMORY_CAPACITY; i++) memory[i] = EMPTY;

  // Executed processes array and their remaining times
  bool executed[processCount];
  int remainingTime[processCount], prevRemainingTime[processCount];
  for (int i = 0; i < processCount; i++) {
    executed[i] = false;
    remainingTime[i] = processes[i].time;
    prevRemainingTime[i] = EMPTY;
  }

  // Hold performance stats
  double maxOverhead = 0.0, totalOverhead = 0.0;

  while (remain > 0) {

    // Avoids accidental quantum skips
    int readyTime = EMPTY, prevRunning = EMPTY;

    for (int i = 0; i < processCount; i++) {

      // Print when processes are ready
      if(memoryChoice) readyProcess(processCount, totalTime, quantum, memory, processes, sjf, 0, &readyTime);

      if (sjf) {
        int shortest = shortestProcess(processes, processCount, totalTime, executed);

        // If none available to execute
        if (shortest == EMPTY) {
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
        executed[shortest] = true;

        remain--;

        updatePerformance(processes, totalTime, shortest, &turnaround, &
          maxOverhead, & totalOverhead);

        if (memoryChoice) {
          readyProcess(processCount, totalTime, quantum, memory, processes, sjf, 1, &readyTime);
          modifyMemory(memory, shortest, processes[shortest].memoryStart, processes[shortest].memory, 0);
        }

        printf("%d,FINISHED,process_name=%s,proc_remaining=%d\n",
          totalTime, processes[shortest].name, lowerTime(totalTime, executed, processes, processCount, quantum));
          
      } else {

        // For best-fit, only start process if it's started
        int startedCheck = 1;
        if (memoryChoice == 1) startedCheck = processes[i].memoryStart != EMPTY;

        // If appropriate arrival and not executed yet
        if (executed[i] == false && processes[i].arrival <= totalTime && startedCheck) {
          totalTime += quantum;

          // Fix quantum skip bug
          if (readyTime != EMPTY && readyTime != totalTime - quantum) {
            totalTime = readyTime + quantum;
            remainingTime[prevProcess] += quantum;
          }

          // Only print first running instance
          if (i != lastExecuted) {

            // Avoid time lag
            if (prevRunning == totalTime - quantum) {
              totalTime += quantum;
              remainingTime[prevProcess] -= quantum;
            }
            if (memoryChoice) {
              if (totalTime - quantum < prevRunning) totalTime = quantum + quantum + prevRunning;
            }
            if (prevRemainingTime[i] == remainingTime[i]) remainingTime[i] -= quantum;

            printf("%d,RUNNING,process_name=%s,remaining_time=%d\n",
              totalTime - quantum, processes[i].name, remainingTime[i]);
            prevProcess = i;
            lastExecuted = i;
            prevRunning = totalTime - quantum;
            prevRemainingTime[i] = remainingTime[i];
          }

          // Finish process when no more remaining time
          remainingTime[i] -= quantum;
          if (remainingTime[i] <= 0) {
            executed[i] = true;
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