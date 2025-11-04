#include "process_manager.h"

// Queue Initialization
void initQueue(ProcessQueue *queue) {
    queue->count = 0;
}

// Add Process

int addProcess(ProcessQueue *queue, int pid, int arrival_time, int burst_time) {
    if (queue->count >= MAX_PROCESSES) {
        printf("Error: Queue is full. Cannot add process %d.\n", pid);
        return -1;
    }

    // Check if PID already exists
    for (int i = 0; i < queue->count; i++) {
        if (queue->processes[i].pid == pid) {
            printf("Error: Process with PID %d already exists.\n", pid);
            return -1;
        }
    }

    Process newProcess;
    newProcess.pid = pid;
    newProcess.arrival_time = arrival_time;
    newProcess.burst_time = burst_time;

    queue->processes[queue->count++] = newProcess;

    printf("Process %d added successfully.\n", pid);
    return 0;
}

// Delete Process by PID
int deleteProcess(ProcessQueue *queue, int pid) {
    if (queue->count == 0) {
        printf("Queue is empty. No process to delete.\n");
        return -1;
    }

    int found = 0;
    for (int i = 0; i < queue->count; i++) {
        if (queue->processes[i].pid == pid) {
            found = 1;
            // Shift remaining processes
            for (int j = i; j < queue->count - 1; j++) {
                queue->processes[j] = queue->processes[j + 1];
            }
            queue->count--;
            printf("Process %d deleted successfully.\n", pid);
            break;
        }
    }

    if (!found) {
        printf("Error: Process with PID %d not found.\n", pid);
        return -1;
    }

    return 0;
}

// Display Queue Contents
void displayQueue(const ProcessQueue *queue) {
    if (queue->count == 0) {
        printf("\nReady queue is empty.\n");
        return;
    }

    printf("\nCurrent Ready Queue:\n");
    printf("----------------------------------\n");
    printf("| PID | Arrival Time | Burst Time |\n");
    printf("----------------------------------\n");

    for (int i = 0; i < queue->count; i++) {
        printf("| %-3d | %-12d | %-10d |\n",
               queue->processes[i].pid,
               queue->processes[i].arrival_time,
               queue->processes[i].burst_time);
    }

    printf("----------------------------------\n");
}


// Get Total Process Count
int getProcessCount(const ProcessQueue *queue) {
    return queue->count;
}


// Output test
#ifdef TEST_PROCESS_MANAGER
int main() {
    ProcessQueue queue;
    initQueue(&queue);

    int choice, pid, arrival, burst;

    printf("===== Process Scheduler =====\n");

    while (1) {
        printf("\nMenu:\n");
        printf("1. Add Process\n");
        printf("2. Delete Process\n");
        printf("3. Display Queue\n");
        printf("4. Show Total Processes\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n'); // clear buffer
            continue;
        }

        switch (choice) {
            case 1:
                printf("Enter PID: ");
                scanf("%d", &pid);
                printf("Enter Arrival Time: ");
                scanf("%d", &arrival);
                printf("Enter Burst Time: ");
                scanf("%d", &burst);
                addProcess(&queue, pid, arrival, burst);
                break;

            case 2:
                printf("Enter PID to delete: ");
                scanf("%d", &pid);
                deleteProcess(&queue, pid);
                break;

            case 3:
                displayQueue(&queue);
                break;

            case 4:
                printf("Total processes in queue: %d\n", getProcessCount(&queue));
                break;

            case 5:
                printf("Exiting...\n");
                return 0;

            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}
#endif
