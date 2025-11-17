# CPU Scheduling Simulator

This project implements and compares five classic CPU scheduling algorithms using clean, efficient, array-based data structures. It simulates how an operating system schedules processes, logs a full execution timeline, and generates detailed performance statistics.

## 📌 Features
- **FCFS** — First-Come First-Served  
- **SJF** — Shortest Job First (Non-Preemptive, Min-Heap)  
- **SRTF** — Shortest Remaining Time First (Preemptive, Min-Heap)  
- **Priority Scheduling** — Preemptive (Min-Heap)  
- **Round Robin** — Circular Queue with time slice  
- Outputs:
  - `process_stats.csv` — final metrics  
  - `execution_order.csv` — CPU timeline  
- Prints final summary:
  - total processes  
  - avg waiting time  
  - avg turnaround time  
  - avg completion time  
  - avg response time  

## 📂 Project Structure
```
├── main.c
├── scheduler.c
├── scheduler.h
├── input.csv
├── process_stats.csv 
└── execution_order.csv 
```

## 🔧 Data Structures Used
| Algorithm | Data Structure | Reason |
|----------|----------------|--------|
| FCFS | FIFO Queue | Maintains arrival order |
| SJF | Min-Heap | Extracts shortest job efficiently |
| SRTF | Min-Heap | Enables preemptive shortest-remaining-time selection |
| Priority | Min-Heap | Quickly selects highest priority |
| Round Robin | Circular Queue | Fair and fast time-slice rotation |

All structures use **static arrays**, not linked lists.

## 📥 Input Format (`input.csv`)
```
pid,arrival,burst,priority
1,0,8,2
2,1,4,1
3,2,9,3
4,3,5,2
```

## ▶️ How to Compile
```
gcc main.c scheduler.c -o scheduler
```

## 🚀 How to Run
```
./scheduler
```

Select a scheduling algorithm from the menu.

## 📤 Output Files
### 1. process_stats.csv
Contains per-process metrics:
- waiting time  
- turnaround time  
- response time  
- completion time  

### 2. execution_order.csv
Contains the CPU timeline:
```
time,pid,event
0,1,start
4,2,preempted
```

## 📘 Summary
This project demonstrates how different CPU scheduling algorithms behave under the same workload.  
Using array-based queues and heaps ensures efficiency, clarity, and predictable performance.  
The simulator produces useful CSV files for analysis and logs a complete execution order for understanding preemption and fairness.
