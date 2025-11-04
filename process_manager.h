#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100


// Structure Definitions ------------------------------------------------------

typedef struct {
    int pid;            // Process ID
    int arrival_time;   // Time process arrives
    int burst_time;     // CPU burst time
} Process;

// Represents the ready queue
typedef struct {
    Process processes[MAX_PROCESSES];
    int count;          // Number of processes in queue
} ProcessQueue;


// Functions -------------------------------------------------------------------

// Initialize the ready queue
void initQueue(ProcessQueue *queue);

// Add a process to the ready queue
int addProcess(ProcessQueue *queue, int pid, int arrival_time, int burst_time);

// Delete a process by PID
int deleteProcess(ProcessQueue *queue, int pid);

// Display all processes currently in the queue
void displayQueue(const ProcessQueue *queue);

// Get the total number of processes in the queue
int getProcessCount(const ProcessQueue *queue);

#endif