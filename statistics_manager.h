#ifndef STATISTICS_MANAGER_H
#define STATISTICS_MANAGER_H

#include "process_manager.h"

typedef struct {
    double avg_waiting_time;
    double avg_turnaround_time;
    int total_processes;
    const char *algorithm;
} SchedulerStats;

// Function to compute and fill all statistics (FCFS)
SchedulerStats computeStatistics(const ProcessQueue *queue);

#endif
