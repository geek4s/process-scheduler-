#include "scheduler.h"

// ===================================================
// 🧩 Queue Operations (Linked List Implementation)
// ===================================================

// Function: enqueue()
// Adds a process to the end of the ready queue.
void enqueue(Queue *q, Process *p) {
    if (!p) return; // Null check
    p->next = NULL;

    if (q->rear == NULL) {
        // Empty queue → first element
        q->front = q->rear = p;
    } else {
        q->rear->next = p;
        q->rear = p;
    }
    q->size++;
}

// Function: dequeue()
// Removes and returns the process at the front of the queue.
Process *dequeue(Queue *q) {
    if (is_empty(q)) return NULL;

    Process *temp = q->front;
    q->front = q->front->next;

    if (q->front == NULL) {
        q->rear = NULL; // Queue becomes empty
    }

    q->size--;
    temp->next = NULL; // Isolate dequeued node
    return temp;
}

// Function: is_empty()
// Checks if the queue is empty.
int is_empty(Queue *q) {
    return (q->front == NULL);
}

// Function: display_queue()
// Displays the contents of the ready queue.
void display_queue(Queue *q) {
    if (is_empty(q)) {
        printf("\n[Queue is Empty]\n");
        return;
    }

    printf("\n🔁 Ready Queue: ");
    Process *temp = q->front;
    while (temp != NULL) {
        printf("[%d|%s] -> ", temp->pid, temp->name);
        temp = temp->next;
    }
    printf("NULL\n");
}

// ===================================================
// 🧩 Process Management
// ===================================================

// Function: create_process()
// Interactively creates a new process and adds it to the list.
void create_process(Process processes[], int *count) {
    if (*count >= MAX_PROCESSES) {
        printf("⚠️  Maximum process limit reached!\n");
        return;
    }

    Process *p = &processes[*count];
    printf("\n--- Create New Process ---\n");

    printf("Enter Process ID: ");
    scanf("%d", &p->pid);

    // Validate duplicate PID
    for (int i = 0; i < *count; i++) {
        if (processes[i].pid == p->pid) {
            printf("❌ Error: Process ID already exists!\n");
            return;
        }
    }

    printf("Enter Process Name: ");
    scanf("%s", p->name);
    printf("Enter Arrival Time: ");
    scanf("%d", &p->arrival_time);
    printf("Enter Burst Time: ");
    scanf("%d", &p->burst_time);
    printf("Enter Priority: ");
    scanf("%d", &p->priority);

    // Initialize process defaults
    p->remaining_time = p->burst_time;
    p->completion_time = 0;
    p->waiting_time = 0;
    p->turnaround_time = 0;
    p->is_completed = 0;
    p->next = NULL;

    (*count)++;
    printf("✅ Process [%d - %s] created successfully!\n", p->pid, p->name);
}

// Function: delete_process()
// Deletes a process with the given PID from the array.
void delete_process(Process processes[], int *count, int pid) {
    if (*count == 0) {
        printf("⚠️  No processes available to delete.\n");
        return;
    }

    int found = 0;
    for (int i = 0; i < *count; i++) {
        if (processes[i].pid == pid) {
            found = 1;
            for (int j = i; j < *count - 1; j++) {
                processes[j] = processes[j + 1];
            }
            (*count)--;
            printf("🗑️  Process with PID %d deleted successfully.\n", pid);
            break;
        }
    }

    if (!found) {
        printf("❌ Error: No process found with PID %d.\n", pid);
    }
}

// ===================================================
// 🧩 CSV File Loading
// ===================================================

// Function: load_processes_from_csv()
// Reads processes from a CSV file formatted as:
// pid,name,arrival_time,burst_time,priority
void load_processes_from_csv(const char *filename, Process processes[], int *count) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("❌ Error: Could not open file '%s'\n", filename);
        return;
    }

    char line[256];
    int line_num = 0;
    *count = 0;

    while (fgets(line, sizeof(line), fp)) {
        // Skip header line if present
        if (line_num == 0 && strstr(line, "pid")) {
            line_num++;
            continue;
        }

        Process *p = &processes[*count];
        int read_fields = sscanf(line, "%d,%[^,],%d,%d,%d",
                                 &p->pid,
                                 p->name,
                                 &p->arrival_time,
                                 &p->burst_time,
                                 &p->priority);

        if (read_fields == 5) {
            // Initialize computed fields
            p->remaining_time = p->burst_time;
            p->completion_time = 0;
            p->waiting_time = 0;
            p->turnaround_time = 0;
            p->is_completed = 0;
            p->next = NULL;
            (*count)++;

            if (*count >= MAX_PROCESSES) break;
        } else {
            printf("⚠️  Skipping invalid line %d: %s", line_num + 1, line);
        }

        line_num++;
    }

    fclose(fp);
    printf("✅ Loaded %d processes from '%s'\n", *count, filename);
}
