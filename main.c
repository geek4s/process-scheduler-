#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"

Process processes[MAX_PROCESSES];
int n;

// =================== LOAD INPUT ===================
// Format: pid,arrival,burst,priority

void load_input() {
    FILE *fp = fopen("input.csv", "r");
    if (!fp) {
        printf("ERROR: Could not open input.csv\n");
        exit(1);
    }

    char line[256];
    n = 0;

    // Skip header
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%d,%d,%d,%d",
               &processes[n].pid,
               &processes[n].arrival,
               &processes[n].burst,
               &processes[n].priority);

        processes[n].remaining = processes[n].burst;
        processes[n].completion = 0;
        processes[n].waiting = 0;
        processes[n].turnaround = 0;
        processes[n].response = -1;
        processes[n].started = 0;

        n++;
    }

    fclose(fp);
}

void print_final_stats(Process p[], int n) {
    double total_wait = 0;
    double total_turn = 0;
    double total_comp = 0;
    double total_resp = 0;

    for (int i = 0; i < n; i++) {
        total_wait += p[i].waiting;
        total_turn += p[i].turnaround;
        total_comp += p[i].completion;
        total_resp += p[i].response;
    }

    printf("\n===== FINAL SUMMARY =====\n");
    printf("Total number of processes: %d\n", n);
    printf("Average waiting time: %.2f\n", total_wait / n);
    printf("Average turnaround time: %.2f\n", total_turn / n);
    printf("Average completion time: %.2f\n", total_comp / n);
    printf("Average response time: %.2f\n", total_resp / n);
    printf("==========================\n\n");
}


// =================== MENU ===================

void print_menu() {
    printf("\n=== CPU SCHEDULER ===\n");
    printf("1. FCFS (FIFO queue)\n");
    printf("2. SJF (Min-heap)\n");
    printf("3. SRTF (Min-heap, preemptive)\n");
    printf("4. Priority (Min-heap, preemptive)\n");
    printf("5. Round Robin (Circular queue)\n");
    printf("Choose scheduler: ");
}

// =================== MAIN ===================

int main() {
    int choice;

    load_input();
    print_menu();
    scanf("%d", &choice);

    reset_processes(processes, n);

    switch (choice) {
        case 1:
            fcfs(processes, n);
            break;
        case 2:
            sjf(processes, n);
            break;
        case 3:
            srtf(processes, n);
            break;
        case 4:
            priority_preemptive(processes, n);
            break;
        case 5:
            round_robin(processes, n);
            break;
        default:
            printf("Invalid option.\n");
            return 1;
    }

   write_process_stats_csv(processes, n);

   printf("\nprocess_stats.csv generated.\n");
   printf("execution_order.csv generated.\n");


   print_final_stats(processes, n);

   return 0;

}
