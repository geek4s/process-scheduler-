# CPU Process Scheduler Simulator

This project is a **Process Scheduler Simulator** that mimics core functionality of an actual OS-level process scheduler.  
It allows users to **create**, **delete**, and **view processes** through both a **C backend** and a **Tkinter-based GUI frontend**.


---

## 🎯 Objective

To simulate an **OS process scheduler**, allowing process management and scheduling visualization through a GUI.  
The scheduler selects the next process based on a *scheduling algorithm** .
---

## ⚙️ Core Functionalities

### 1. 🧩 Create Process
- Accepts:
  - **Process ID (PID)**
  - **Arrival Time**
  - **Burst Time**
- Adds the process to the **A Ready Queue**.

### 2. 🗑️ Delete Process
- Removes a process from the queue using its **PID**.

### 3. 📊 Show Statistics
Displays key scheduler information:
- Processes currently in the queue
- **Average Waiting Time**
- **Average Turnaround Time**
- **CPU Utilization** (optional)
- **Algorithm Used**
- **Total Processes Scheduled**

---

## 🖥️ GUI Interface (Tkinter)

### Components:
- **Process Creation Panel:**  
  Input fields for PID, Arrival Time, Burst Time → `[Add Process]` button
- **Process Deletion Panel:**  
  Input field for PID → `[Delete Process]` button
- **Live Process Table:**  
  Displays all processes in the queue (`ttk.Treeview`)
- **Statistics Section:**  
  - Algorithm Used  
  - Total Processes Scheduled  
  - Average Waiting Time  
  - Average Turnaround Time  
- **Clock Label:**  
  Displays current simulation time.

---



