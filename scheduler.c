#include "scheduler.h"
#include <stdlib.h>
#include <string.h>
static FILE *exec_fp = NULL;

/* =============== FIFO QUEUE for FCFS ======================= */

void fifo_init(FIFOQueue *q) {
    q->front = 0;
    q->rear = 0;
}

int fifo_empty(FIFOQueue *q) {
    return q->front == q->rear;
}

void fifo_push(FIFOQueue *q, int value) {
    q->data[q->rear++] = value;
}

int fifo_pop(FIFOQueue *q) {
    return q->data[q->front++];
}

/* =============== CIRCULAR QUEUE for ROUND ROBIN ========================= */

void cq_init(CircularQueue *q) {
    q->front = 0;
    q->rear = 0;
    q->count = 0;
}

int cq_empty(CircularQueue *q) {
    return q->count == 0;
}

void cq_push(CircularQueue *q, int value) {
    q->data[q->rear] = value;
    q->rear = (q->rear + 1) % MAX_PROCESSES;
    q->count++;
}

int cq_pop(CircularQueue *q) {
    int v = q->data[q->front];
    q->front = (q->front + 1) % MAX_PROCESSES;
    q->count--;
    return v;
}


/* ==================== MIN HEAP for SJF/SRTF/PRIORITY ================*/

static void heap_swap(MinHeap *h, int a, int b) {
    int tmp_idx = h->idx[a];
    int tmp_key = h->key[a];

    h->idx[a] = h->idx[b];
    h->key[a] = h->key[b];

    h->idx[b] = tmp_idx;
    h->key[b] = tmp_key;
}

void heap_init(MinHeap *h) {
    h->size = 0;
}

void heap_push(MinHeap *h, int process_index, int key) {
    int i = h->size++;
    h->idx[i] = process_index;
    h->key[i] = key;

    while (i != 0) {
        int parent = (i - 1) / 2;
        if (h->key[parent] <= h->key[i])
            break;
        heap_swap(h, parent, i);
        i = parent;
    }
}

int heap_pop(MinHeap *h) {
    if (h->size == 0)
        return -1;

    int ret = h->idx[0];

    h->size--;
    h->idx[0] = h->idx[h->size];
    h->key[0] = h->key[h->size];

    int i = 0;

    while (1) {
        int left = 2*i + 1;
        int right = 2*i + 2;
        int smallest = i;

        if (left < h->size && h->key[left] < h->key[smallest])
            smallest = left;
        if (right < h->size && h->key[right] < h->key[smallest])
            smallest = right;

        if (smallest == i)
            break;

        heap_swap(h, i, smallest);
        i = smallest;
    }

    return ret;
}

void heap_update_key(MinHeap *h, int process_index, int newkey) {
    for (int i = 0; i < h->size; i++) {
        if (h->idx[i] == process_index) {
            h->key[i] = newkey;

            int cur = i;
            while (cur != 0) {
                int parent = (cur - 1) / 2;
                if (h->key[parent] <= h->key[cur])
                    break;
                heap_swap(h, parent, cur);
                cur = parent;
            }

            while (1) {
                int left = 2*cur + 1;
                int right = 2*cur + 2;
                int small = cur;

                if (left < h->size && h->key[left] < h->key[small])
                    small = left;
                if (right < h->size && h->key[right] < h->key[small])
                    small = right;

                if (small == cur)
                    break;

                heap_swap(h, cur, small);
                cur = small;
            }

            break;
        }
    }
}


/* ===================== CSV OUTPUT ============================*/

void write_process_stats_csv(Process p[], int n) {
    FILE *fp = fopen("process_stats.csv", "w");
    fprintf(fp, "pid,arrival,burst,completion,priority,waiting,turnaround,response\n");

    for (int i = 0; i < n; i++) {
        fprintf(fp, "%d,%d,%d,%d,%d,%d,%d,%d\n",
                p[i].pid, p[i].arrival, p[i].burst, p[i].completion,
                p[i].priority, p[i].waiting, p[i].turnaround, p[i].response);
    }

    fclose(fp);
}

void write_execution_order_csv(int time, int pid, const char *event) {
    if (!exec_fp) {
        exec_fp = fopen("execution_order.csv", "w");
        fprintf(exec_fp, "time,pid,event\n");
    }
    fprintf(exec_fp, "%d,%d,%s\n", time, pid, event);
}


/* ===================== RESET + STATS ========================= */

void reset_processes(Process p[], int n) {
    for (int i = 0; i < n; i++) {
        p[i].remaining = p[i].burst;
        p[i].completion = 0;
        p[i].waiting = 0;
        p[i].turnaround = 0;
        p[i].response = -1;
        p[i].started = 0;
    }

    if (exec_fp) {
        fclose(exec_fp);
        exec_fp = NULL;
    }
}

void calculate_final_stats(Process p[], int n) {
    for (int i = 0; i < n; i++) {
        p[i].turnaround = p[i].completion - p[i].arrival;
        p[i].waiting = p[i].turnaround - p[i].burst;
        if (p[i].response < 0)
            p[i].response = p[i].waiting;
    }
}

/*========================== FCFS ============================= */

void fcfs(Process p[], int n) {
    FIFOQueue q;
    fifo_init(&q);

    int time = 0;
    int completed = 0;

    // Load processes in arrival order
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n - 1; j++)
            if (p[j].arrival > p[j+1].arrival) {
                Process tmp = p[j];
                p[j] = p[j+1];
                p[j+1] = tmp;
            }

    int next = 0;

    while (completed < n) {
        // enqueue all processes arriving at this time
        while (next < n && p[next].arrival <= time) {
            fifo_push(&q, next);
            next++;
        }

        if (fifo_empty(&q)) {
            time++; 
            continue;
        }

        int idx = fifo_pop(&q);

        write_execution_order_csv(time, p[idx].pid, "start");

        time += p[idx].burst;
        p[idx].completion = time;

        write_execution_order_csv(time, p[idx].pid, "finish");

        completed++;
    }

    calculate_final_stats(p, n);
}


/* ========================== SJF ==============================
   ============ Non-preemptive, heap-based ===================== */

void sjf(Process p[], int n) {
    MinHeap h;
    heap_init(&h);

    int time = 0;
    int completed = 0;
    int arrived = 0;

    // Sort by arrival time
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n - 1; j++)
            if (p[j].arrival > p[j+1].arrival) {
                Process tmp = p[j];
                p[j] = p[j+1];
                p[j+1] = tmp;
            }

    while (completed < n) {

        while (arrived < n && p[arrived].arrival <= time) {
            heap_push(&h, arrived, p[arrived].burst);
            arrived++;
        }

        if (h.size == 0) { 
            time++; 
            continue; 
        }

        int idx = heap_pop(&h);

        write_execution_order_csv(time, p[idx].pid, "start");

        time += p[idx].burst;
        p[idx].completion = time;

        write_execution_order_csv(time, p[idx].pid, "finish");

        completed++;
    }

    calculate_final_stats(p, n);
}

/* ============================ SRTF ===========================
   ============ Preemptive SJF using a real min-heap =========== */

void srtf(Process p[], int n) {
    MinHeap h;
    heap_init(&h);

    int time = 0;
    int completed = 0;
    int arrived = 0;
    int last_running = -1;

    // Sort by arrival time
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n - 1; j++)
            if (p[j].arrival > p[j+1].arrival) {
                Process tmp = p[j];
                p[j] = p[j+1];
                p[j+1] = tmp;
            }

    while (completed < n) {

        // push newly arrived processes (key = remaining time)
        while (arrived < n && p[arrived].arrival <= time) {
            heap_push(&h, arrived, p[arrived].remaining);
            arrived++;
        }

        if (h.size == 0) {
            time++;
            continue;
        }

        // get process with least remaining time
        int idx = heap_pop(&h);

        // Log start or preemption switch
        if (idx != last_running) {
            write_execution_order_csv(time, p[idx].pid, "preempted switch");
        }

        last_running = idx;

        // First time response
        if (!p[idx].started) {
            p[idx].started = 1;
            p[idx].response = time - p[idx].arrival;
        }

        // Run for 1 time unit
        p[idx].remaining--;
        time++;

        // Re-insert if not done
        if (p[idx].remaining > 0) {

            // New arrivals during run (important!)
            while (arrived < n && p[arrived].arrival <= time) {
                heap_push(&h, arrived, p[arrived].remaining);
                arrived++;
            }

            // Push this process back with new remaining time
            heap_push(&h, idx, p[idx].remaining);

        } else {
            // Finished
            p[idx].completion = time;
            completed++;

            write_execution_order_csv(time, p[idx].pid, "finish");
        }
    }

    calculate_final_stats(p, n);
}

/* ===================== PRIORITY PREEMPTIVE ===================
   ============= Using a min-heap keyed by priority ============= */

void priority_preemptive(Process p[], int n) {
    MinHeap h;
    heap_init(&h);

    int time = 0;
    int completed = 0;
    int arrived = 0;
    int last_running = -1;

    // Sort by arrival to load processes in correct order
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n - 1; j++)
            if (p[j].arrival > p[j+1].arrival) {
                Process tmp = p[j];
                p[j] = p[j+1];
                p[j+1] = tmp;
            }

    while (completed < n) {

        // Add newly arrived processes to heap (key = priority)
        while (arrived < n && p[arrived].arrival <= time) {
            heap_push(&h, arrived, p[arrived].priority);
            arrived++;
        }

        if (h.size == 0) {
            time++;
            continue;
        }

        // Get process with best (lowest) priority value
        int idx = heap_pop(&h);

        // Check for switch/preemption
        if (idx != last_running) {
            write_execution_order_csv(time, p[idx].pid, "preempted switch");
        }
        last_running = idx;

        // First-time response
        if (!p[idx].started) {
            p[idx].started = 1;
            p[idx].response = time - p[idx].arrival;
        }

        // Execute for 1 time unit
        p[idx].remaining--;
        time++;

        // New arrivals during this time
        while (arrived < n && p[arrived].arrival <= time) {
            heap_push(&h, arrived, p[arrived].priority);
            arrived++;
        }

        // If not finished, push back with same key (priority)
        if (p[idx].remaining > 0) {
            heap_push(&h, idx, p[idx].priority);
        } else {
            p[idx].completion = time;
            completed++;
            write_execution_order_csv(time, p[idx].pid, "finish");
        }
    }

    calculate_final_stats(p, n);
}

/* ========================= ROUND ROBIN =======================
   ================= Circular Queue Implementation ============== */

void round_robin(Process p[], int n) {
    CircularQueue q;
    cq_init(&q);

    int time = 0;
    int completed = 0;
    int arrived = 0;
    int last_running = -1;

    // Sort by arrival time
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n - 1; j++)
            if (p[j].arrival > p[j+1].arrival) {
                Process tmp = p[j];
                p[j] = p[j+1];
                p[j+1] = tmp;
            }

    // Load processes that arrive at time 0
    while (arrived < n && p[arrived].arrival <= time) {
        cq_push(&q, arrived);
        arrived++;
    }

    while (completed < n) {

        // If queue empty, advance time until something arrives
        if (cq_empty(&q)) {
            time++;
            while (arrived < n && p[arrived].arrival <= time) {
                cq_push(&q, arrived);
                arrived++;
            }
            continue;
        }

        int idx = cq_pop(&q);

        // Log start or RR switch
        if (idx != last_running) {
            write_execution_order_csv(time, p[idx].pid, "start or RR switch");
        }
        last_running = idx;

        // First-time response
        if (!p[idx].started) {
            p[idx].started = 1;
            p[idx].response = time - p[idx].arrival;
        }

        int run = (p[idx].remaining < TIME_SLICE ? p[idx].remaining : TIME_SLICE);

        // Execute for 'run' units
        for (int i = 0; i < run; i++) {
            p[idx].remaining--;
            time++;

            // Load new arrivals every time unit
            while (arrived < n && p[arrived].arrival <= time) {
                cq_push(&q, arrived);
                arrived++;
            }

            if (p[idx].remaining == 0)
                break;
        }

        if (p[idx].remaining == 0) {
            p[idx].completion = time;
            completed++;
            write_execution_order_csv(time, p[idx].pid, "finish");
        } else {
            // Process re-enters RR queue
            cq_push(&q, idx);
        }
    }

    calculate_final_stats(p, n);
}
