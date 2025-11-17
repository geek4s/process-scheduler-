#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdio.h>

#define MAX_PROCESSES 100
#define TIME_SLICE 4

// ===================== PROCESS STRUCT =====================
typedef struct {
    int pid;
    int arrival;
    int burst;
    int remaining;
    int completion;
    int priority;

    int waiting;
    int turnaround;
    int response;
    int started;
} Process;


// ===================== FIFO QUEUE (FCFS) =====================
typedef struct {
    int data[MAX_PROCESSES];
    int front;
    int rear;
} FIFOQueue;

void fifo_init(FIFOQueue *q);
int fifo_empty(FIFOQueue *q);
void fifo_push(FIFOQueue *q, int value);
int fifo_pop(FIFOQueue *q);


// ===================== CIRCULAR QUEUE (ROUND ROBIN) =====================
typedef struct {
    int data[MAX_PROCESSES];
    int front;
    int rear;
    int count;
} CircularQueue;

void cq_init(CircularQueue *q);
int cq_empty(CircularQueue *q);
void cq_push(CircularQueue *q, int value);
int cq_pop(CircularQueue *q);


// ===================== MIN HEAP (ARRAY) =====================
// Used for SJF, SRTF, Priority Scheduling

typedef struct {
    int size;
    int idx[MAX_PROCESSES];  
    int key[MAX_PROCESSES];
} MinHeap;

void heap_init(MinHeap *h);
void heap_push(MinHeap *h, int process_index, int key);
int heap_pop(MinHeap *h);
void heap_update_key(MinHeap *h, int process_index, int newkey);


// ===================== CSV OUTPUTS =====================

void write_process_stats_csv(Process p[], int n);
void write_execution_order_csv(int time, int pid, const char *event);


// ===================== RESET + STATS =====================

void reset_processes(Process p[], int n);
void calculate_final_stats(Process p[], int n);


// ===================== SCHEDULERS =====================

void fcfs(Process p[], int n);
void sjf(Process p[], int n);
void srtf(Process p[], int n);
void priority_preemptive(Process p[], int n);
void round_robin(Process p[], int n);

#endif
