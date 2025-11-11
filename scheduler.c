#include "scheduler.h"

// ========================
// 🧠 Queue Operations
// ========================

// Enqueue a process to the ready queue
void enqueue(Queue *q, Process *p) {
    if (!p) return;
    p->next = NULL;

    if (q->rear == NULL) { // empty queue
        q->front = q->rear = p;
    } else {
        q->rear->next = p;
        q->rear = p;
    }
    q->size++;
}

// Dequeue (remove) a process from the front
Process* dequeue(Queue *q) {
    if (is_empty(q)) return NULL;

    Process *temp = q->front;
    q->front = q->front->next;

    if (q->front == NULL) q->rear = NULL;

    q->size--;
    temp->next = NULL;
    return temp;
}

// Check if queue is empty
int is_empty(Queue *q) {
    return (q->front == NULL);
}

// Display queue contents
void display_queue(Queue *q) {
    if (is_empty(q)) {
        printf("\n[Queue Empty]\n");
        return;
    }

    printf("\nReady Queue: ");
    Process *temp = q->front;
    while (temp) {
        printf("[%d|%s] -> ", temp->pid, temp->name);
        temp = temp->next;
    }
    printf("NULL\n");
}

// ========================
// 🧠 Process Management
// ========================

// Create a new process interactively
void create_process(Process processes[], int *count) {
    if (*count >= MAX_PROCESSES) {
        printf("⚠️ Process limit reached!\n");
        return;
    }

    Process *p = &processes[*count];

    printf("\nEnter Process ID: ");
    scanf("%d", &p->pid);

    // Check duplicate PID
    for (int i = 0; i < *count; i++) {
        if (processes[i].pid == p->pid) {
            printf("❌ Error: PID already exists!\n");
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

    // Initialize computed fields
    p->remaining_time = p->burst_time;
    p->completion_time = 0;
    p->waiting_time = 0;
    p->turnaround_time = 0;
    p->is_completed = 0;
    p->next = NULL;

    (*count)++;
    printf("✅ Process created successfully!\n");
}

// Delete a process by PID
void delete_process(Process processes[], int *count, int pid) {
    int found = 0;
    for (int i = 0; i < *count; i++) {
        if (processes[i].pid == pid) {
            found = 1;
            // Shift elements
            for (int j = i; j < *count - 1; j++) {
                processes[j] = processes[j + 1];
            }
            (*count)--;
            printf("🗑️ Process with PID %d deleted.\n", pid);
            break;
        }
    }
    if (!found) {
        printf("❌ No process found with PID %d.\n", pid);
    }
}

// ========================
// 🧠 CSV Loading
// ========================

void load_processes_from_csv(const char *filename, Process processes[], int *count) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("❌ Error: Cannot open file %s\n", filename);
        return;
    }

    char line[256];
    int line_number = 0;
    *count = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (line_number == 0 && strstr(line, "pid")) {
            // Skip header line
            line_number++;
            continue;
        }

        Process *p = &processes[*count];
        if (sscanf(line, "%d,%[^,],%d,%d,%d",
                   &p->pid, p->name,
                   &p->arrival_time,
                   &p->burst_time,
                   &p->priority) == 5) {

            p->remaining_time = p->burst_time;
            p->completion_time = 0;
            p->waiting_time = 0;
            p->turnaround_time = 0;
            p->is_completed = 0;
            p->next = NULL;
            (*count)++;

            if (*count >= MAX_PROCESSES) break;
        } else {
            printf("⚠️ Skipping invalid line %d: %s", line_number + 1, line);
        }
        line_number++;
    }

    fclose(fp);
    printf("✅ Loaded %d processes from %s\n", *count, filename);
}
