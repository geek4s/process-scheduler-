#include "statistics_manager.h"
#include <stdio.h>

SchedulerStats computeStatistics(const ProcessQueue *queue) {
    SchedulerStats stats = {0.0, 0.0, 0, "FCFS"};

    if (queue->count == 0) {
        printf("No processes in queue to compute statistics.\n");
        return stats;
    }

    int n = queue->count;
    double waiting_time[n];
    double turnaround_time[n];

    int current_time = 0;

    for (int i = 0; i < n; i++) {
        Process p = queue->processes[i];

        if (current_time < p.arrival_time)
            current_time = p.arrival_time; // idle until next arrival

        waiting_time[i] = current_time - p.arrival_time;
        current_time += p.burst_time;
        turnaround_time[i] = waiting_time[i] + p.burst_time;
    }

    double total_wait = 0, total_turn = 0;
    for (int i = 0; i < n; i++) {
        total_wait += waiting_time[i];
        total_turn += turnaround_time[i];
    }

    stats.avg_waiting_time = total_wait / n;
    stats.avg_turnaround_time = total_turn / n;
    stats.total_processes = n;

    return stats;
}

// Small test main (for debugging)
// Compile: gcc statistics_manager.c process_manager.c -DTEST_STATS -o test_stats
#ifdef TEST_STATS
int main() {
    ProcessQueue q;
    initQueue(&q);

    addProcess(&q, 1, 0, 4);
    addProcess(&q, 2, 1, 3);
    addProcess(&q, 3, 2, 2);
    addProcess(&q, 4, 3, 1);

    displayQueue(&q);

    SchedulerStats s = computeStatistics(&q);
    printf("\nAlgorithm: %s\n", s.algorithm);
    printf("Total Processes: %d\n", s.total_processes);
    printf("Average Waiting Time: %.2f\n", s.avg_waiting_time);
    printf("Average Turnaround Time: %.2f\n", s.avg_turnaround_time);

    return 0;
}
#endif
