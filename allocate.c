// Defined helper function and structs
#include "helper.h"
#include "process.h"

void scheduler(Process processes[], int processCount, 
        int memoryChoice, int quantum, bool sjf);

int main(int argc, char **argv)
{
	
    // Retrieve arguments and file
    Arguments args;
	if (!parseArguments(argc, argv, &args)) return 1;

	FILE *processesFile = fopen(args.file, "r");
	if (processesFile == NULL) return 1;

	Process processes[MAX_PROCESSES];
	int processesCount = 0;

	Process p;
	p.memoryStart = EMPTY;
	while (fscanf(processesFile, "%d %s %d %d", &p.arrival, 
            p.name, &p.time, &p.memory) == NUM_ARGS) 
            processes[processesCount++] = p;

	fclose(processesFile);

    // Sort processes by arrival, time, then name
	qsort(processes, processesCount, sizeof(Process), 
            compareProcess);

	scheduler(processes, processesCount, args.memoryChoice, 
            args.quantum, !args.scheduleChoice);

	return 0;
}

/* Scheduler implementation. Note that separating SJF and RR
 * is unnecessary, as they rely on the outer for loop
 */
void scheduler(Process processes[], int processCount, 
        int memoryChoice, int quantum, bool scheduleChoice)
{
	int totalTime = 0, lastExecuted = EMPTY, turnaround = 0, 
            prevProcess = 0, memory[MEMORY_CAPACITY], 
            remain = processCount;

    // Initialising memory
	for (int i = 0; i < MEMORY_CAPACITY; i++) 
            memory[i] = EMPTY;

	// Executed processes array and their remaining times
	bool executed[processCount];
	int remainingTime[processCount], 
            prevRemainingTime[processCount];
	for (int i = 0; i < processCount; i++)
	{
		executed[i] = false;
		remainingTime[i] = processes[i].time;
		prevRemainingTime[i] = EMPTY;
	}

	// Hold performance stats
	double maxOverhead = 0.0, totalOverhead = 0.0;

	while (remain > 0)
	{
		// Prevents quantum skips
		int readyTime = EMPTY, prevRunning = EMPTY;

		for (int i = 0; i < processCount; i++)
		{
			// Print when processes are ready
			if (memoryChoice) readyProcess(processCount, 
                    totalTime, quantum, memory, processes, 
                    scheduleChoice, false, &readyTime);

            // Shortest job first
			if (scheduleChoice)
			{
				// If no shortest process available, skip
                int shortest = shortestProcess(processes, 
                        processCount, totalTime, executed);
				if (shortest == EMPTY)
				{
					totalTime++;
					continue;
				}

				printf("%d,RUNNING,process_name=%s,"
                        "remaining_time=%d\n", totalTime, 
                        processes[shortest].name, 
                        processes[shortest].time);

				// Add quantums passeds, execute shortest
				int quantums = 0;
				while (quantums < processes[shortest].time)
                        quantums += quantum;
				totalTime += quantums;
				executed[shortest] = true;
				remain--;

				updatePerformance(processes, totalTime, 
                        shortest, &turnaround, &maxOverhead, 
                        &totalOverhead);

				if (memoryChoice)
				{
					readyProcess(processCount, totalTime, 
                            quantum, memory, processes, 
                            scheduleChoice, true, 
                            &readyTime);
					modifyMemory(memory, shortest, 
                            processes[shortest].memoryStart, 
                            processes[shortest].memory, 
                            false);
				}

				printf("%d,FINISHED,process_name=%s,"
                        "proc_remaining=%d\n", totalTime, 
                        processes[shortest].name, 
                        remaining(totalTime, executed, 
                                processes, processCount, 
                                quantum));
			}
			else // Round robin
			{
			 	/* For best-fit, 
                 * only start process if it's assigned
                 */
				int startedCheck = 1;
				if (memoryChoice == true) startedCheck = 
                        processes[i].memoryStart != EMPTY;

				// If appropriate arrival and not executed
				if (executed[i] == false && 
                        processes[i].arrival <= totalTime && 
                        startedCheck)
				{
					totalTime += quantum;

					// Fix quantum skip by adding quantum
					if (readyTime != EMPTY && 
                            readyTime != totalTime - quantum)
					{
						totalTime = readyTime + quantum;
						remainingTime[prevProcess] += 
                                quantum;
					}

					// Only print first running instance
					if (i != lastExecuted)
					{
					 	// Avoid quantum skip
						if (prevRunning == 
                                totalTime - quantum)
						{
							totalTime += quantum;
							remainingTime[prevProcess] -= 
                                    quantum;
						}

                        // Avoids multiple running same time
						if (memoryChoice)
						{
							if (totalTime - quantum < 
                                    prevRunning) 
                                    totalTime = 2 * quantum
                                            + prevRunning;
						}
						if (prevRemainingTime[i] == 
                                remainingTime[i]) 
                                remainingTime[i] -= quantum;

                        // Print then update running process
						printf("%d,RUNNING,process_name=%s,"
                                "remaining_time=%d\n",
							    totalTime - quantum, 
                                processes[i].name, 
                                remainingTime[i]);
						prevProcess = i;
						lastExecuted = i;
						prevRunning = totalTime - quantum;
						prevRemainingTime[i] = 
                                remainingTime[i];
					}

					remainingTime[i] -= quantum;
					if (remainingTime[i] <= 0)
					{
                        // Indicate executed
						executed[i] = true;
						remain--;

						updatePerformance(processes, 
                                totalTime, i, &turnaround, 
                                &maxOverhead, &totalOverhead);

						printf("%d,FINISHED,process_name=%s,"
                                "proc_remaining=%d\n",
							    totalTime, processes[i].name, 
                                remain);

						/* End current iteration to avoid 
                         * incorrect printing order
                         */
						if (memoryChoice)
						{
							modifyMemory(memory, i, 
                                    processes[i].memoryStart, 
                                    processes[i].memory, 
                                    false);
							break;
						}
					}
				}
			}
		}
	}

	printPerformance(turnaround, maxOverhead, 
            totalOverhead, processCount);
	printf("Makespan %d\n", totalTime);
}