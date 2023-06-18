# Simulated C Process Scheduler

This repository contains a process scheduler simulator written in C. The program simulates how different process scheduling algorithms, specifically Shortest Job First (SJF) and Round Robin (RR), handle a list of processes. It also simulates memory management based on user-specified parameters.

## Table of Contents

1. [Features](#Features)
2. [Getting Started](#Getting-Started)
   - [Prerequisites](#Prerequisites)
   - [Installation](#Installation)
3. [Usage](#Usage)

## Features

- **Simulated Scheduling Algorithms**: The program simulates both the SJF and RR scheduling algorithms.
- **Memory Management**: The simulator also handles memory management for each process.
- **Performance Metrics**: At the end of the simulation, the program prints out various performance metrics such as makespan, overhead, and turnaround time.

## Getting Started

### Prerequisites

- You need to have GCC, a compiler for C and C++, installed on your system to compile and run this program.

### Installation

1. Clone this repository using git:

```
git clone https://github.com/shaneSleeman/process-scheduler.git
```

2. Change to the project's directory:

```
cd process-scheduler
```

3. Compile the program:

```
gcc -o scheduler main.c helper.c process.c
```

## Usage

To run the process scheduler simulator, you'll need to provide various command line arguments. An example execution would look like:

```
./scheduler processes.txt 1 4 0
```

Where:

- `processes.txt` is the file containing the list of processes. Each line should contain the arrival time, process name, burst time, and memory requirement for a process, separated by spaces.
- `1` is the memory choice. `0` for first-fit and `1` for best-fit.
- `4` is the quantum for the RR algorithm.
- `0` is the scheduling choice. `0` for RR and `1` for SJF.

The program will then output the scheduling decisions and performance metrics to the console.
