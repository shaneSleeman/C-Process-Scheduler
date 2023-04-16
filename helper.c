#include "helper.h"
#include "process.h"

// Retrieve non-executed processes remaining
int remaining(int totalTime, bool executed[], 
        Process processes[], int processCount, int quantum)
{
    int n = 0, atLeast = totalTime - quantum;
	for (int i = 1; i < processCount; i++)
	{
		if (processes[i].arrival < atLeast &&
			!executed[i]) n++;
	}

	return n;
}

// Finds index of shortest remaining (non-executed) process
int shortestProcess(Process processes[], int processCount, 
        int totalTime, bool executed[])
{
	int shortest = EMPTY, minimum = INT_MAX;

	for (int i = 0; i < processCount; i++)
	{
		
        // If shortest so far and arrived
        if (!executed[i] &&
			processes[i].time < minimum &&
			processes[i].arrival <= totalTime)
		{
			shortest = i;
			minimum = processes[i].time;
		}
	}

	return shortest;
}

// Update performance stats
void updatePerformance(Process processes[], int totalTime, 
        int process, int *turnaround,
	double *maxOverhead, double *totalOverhead)
{
	// Current process turnaround and time overhead
	int processTurnaround = totalTime - 
            processes[process].arrival;

	// Update turnaround
	*turnaround += processTurnaround;

	// Current process overhead
	double processOverhead = processTurnaround / 
            (double) processes[process].time;

	// Track max overhead and update total overhead
	if (processOverhead > *maxOverhead) 
            *maxOverhead = processOverhead;
	*totalOverhead += processOverhead;
}

void printPerformance(int turnaround, double maxOverhead, 
        double totalOverhead, int processCount)
{
	printf("Turnaround time %d\n", 
            (int) ceil(turnaround / (double) processCount));
	printf("Time overhead %.2f %.2f\n", 
            round(maxOverhead *PRECISION) / PRECISION, 
            round(totalOverhead / processCount *PRECISION) / 
            PRECISION);
}

// Assign memory start to start+length, to process
void modifyMemory(int memory[], int i, int start, 
        int length, bool fill)
{
	for (int j = start; j < start + length; j++) 
            memory[j] = fill ? i : EMPTY;
}

// Returns quantum at or immediately after time n
int lowestMultiple(int n, int i)
{
	return ((n + i - 1) / i) * i;
}

// Best-fit memory location for a given memory size
int nextFree(int memory[], Process processes[], 
        int processCount, int length)
{
	int currentLocation = EMPTY, tally = 0, minGap = INT_MAX;

	for (int i = 0; i < MEMORY_CAPACITY; i++)
	{
		// Reset tally count if encountered used memory
        if (memory[i] != EMPTY) tally = 0;

        // Else update location and tally
		else
		{
			if (!tally) currentLocation = i;
			tally++;
		}

        // Track smallest gap, return perfect fit
		if (tally >= length)
		{
			int gap = tally - length;

			if (gap < minGap)
			{
				minGap = gap;

				// If gap is perfect
				if (!minGap) return currentLocation;
			}
		}
	}

	// If no perfect fit, return best-fit i.e. smallest gap
	if (minGap != INT_MAX) return currentLocation;

	// When no free location
	return EMPTY;
}

// Assign memory to ready processes, and print them
void readyProcess(int processCount, int totalTime, 
        int quantum, int memory[], Process processes[], 
        bool scheduleChoice, bool offset, int *readyTime)
{
	for (int i = 0; i < processCount; i++)
	{
		// Different arrival for round robin
        int isRR = scheduleChoice ? true : 
                (nextFree(memory, processes, processCount, 
                processes[i].memory) != EMPTY),
			arrivalQuantum = lowestMultiple(
                        processes[i].arrival, quantum);

		// Assign memory if unassigned
        if (processes[i].memoryStart == EMPTY &&
                ((offset && totalTime - quantum >= 
                arrivalQuantum) || (!offset && totalTime >= 
                arrivalQuantum && isRR)))
		{
            // Modify memory and print if free space 
			int freeMemoryIndex = nextFree(memory, processes, 
                    processCount, processes[i].memory);

			if (freeMemoryIndex != EMPTY)
			{
				processes[i].memoryStart = freeMemoryIndex;
				modifyMemory(memory, i, 
                        processes[i].memoryStart, 
                        processes[i].memory, true);
				int printTime = offset ? arrivalQuantum : 
                        lowestMultiple(totalTime, quantum);

				printf("%d,READY,process_name=%s,"
                        "assigned_at=%d\n", printTime, 
                        processes[i].name, 
                        processes[i].memoryStart);

				if (!offset) *readyTime = totalTime;
			}
		}
	}
}

// Process ordering, by name after arrival and times
int compareProcess(const void *a, const void *b)
{
	const Process *processA = (const Process *) a,
		*processB = (const Process *) b;

	if (processA->arrival != processB->arrival) 
            return processA->arrival - processB->arrival;

	if (processA->time != processB->time) 
            return processA->time - processB->time;

	return strcmp(processA->name, processB->name);
}

// Only allows valid arguments
bool parseArguments(int argc, char **argv, Arguments *args)
{
	args->file = NULL;
	args->scheduleChoice = true;
	args->memoryChoice = false;
	args->quantum = MIN_QUANTUM;

	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-f"))
		{
			if (i + 1 < argc) args->file = argv[++i];
			else return false;
		}
		else if (!strcmp(argv[i], "-s"))
		{
			if (i + 1 < argc && !strcmp(argv[i + 1], "RR"))
			{
				args->scheduleChoice = false;
				i++;
			}
			else if (i + 1 < argc && 
                    !strcmp(argv[i + 1], "SJF")) i++;
			else return false;
		}
		else if (!strcmp(argv[i], "-m"))
		{
			if (i + 1 < argc && !strcmp(argv[i + 1], 
                    "best-fit"))
			{
				args->memoryChoice = true;
				i++;
			}
			else if (i + 1 < argc && !strcmp(argv[i + 1], 
                    "infinite")) i++;
			else return false;
		}
		else if (!strcmp(argv[i], "-q"))
		{
			if (i + 1 < argc && atoi(argv[i + 1]) >= 
                    MIN_QUANTUM && atoi(argv[i + 1]) <= 
                    MAX_QUANTUM) 
                    args->quantum = atoi(argv[++i]);
			else return false;
		}
	}

	return args->file != NULL;
}