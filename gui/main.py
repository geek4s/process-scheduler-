#!/usr/bin/env python3
"""
main_gui.py
Tkinter frontend for ProcessScheduler project (Nishitha's part)

This file was updated to handle environments that do not have the `tkinter` module
installed. If `tkinter` is available the normal GUI runs. If not, the script falls
back to a console/CLI demo mode so you can still test the C/Python backend and see
example operations (add/delete/process listing/stats) without a GUI.

Changes made:
- Wrapped tkinter imports in try/except and provide a CLI fallback when missing.
- Added an automated demo/test sequence that runs when tkinter is unavailable.
- Kept the original GUI code intact when tkinter is present.
- Added more test cases in CLI demo to satisfy the "ALWAYS add more test cases" rule.

Run (GUI mode):
    python3 main_gui.py

Run (CLI fallback): If your environment lacks tkinter (common in headless/sandboxed
containers), the script will automatically run CLI demo and exit.

NOTE: If you want the GUI to talk to a real C library, place a shared library at
lib/libprocess.so with the expected functions (init_queue, add_process,
delete_process, get_process_count, get_process_list, compute_stats). If names differ
provide a thin shim or tell me the exact signatures and I'll update this wrapper.

If behavior is unclear (scheduling algorithm choices, tie-breaking rules), please
reply in chat with the expected behavior and I'll adapt the GUI/compute logic.
"""

import os
import sys
import ctypes
import time
from ctypes import c_int, c_double, POINTER, byref

# Try to import tkinter; if unavailable, fall back to CLI demo
try:
    import tkinter as tk
    from tkinter import ttk, messagebox
    TK_AVAILABLE = True
except ModuleNotFoundError:
    TK_AVAILABLE = False
    tk = None
    ttk = None
    messagebox = None
    print("\n*** tkinter not found in this environment — GUI mode disabled.\n    The script will run a CLI/demo sequence instead.\n")

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
LIB_PATH = os.path.join(PROJECT_ROOT, 'lib', 'libprocess.so')

# ---------------------- Backend wrapper ----------------------
class BackendInterface:
    """Abstracts access to C shared library. If lib missing, uses Python fallback."""
    def __init__(self):
        self.lib = None
        self.use_c = False
        if os.path.exists(LIB_PATH):
            try:
                self.lib = ctypes.CDLL(LIB_PATH)
                self._setup_prototypes()
                if hasattr(self.lib, 'init_queue'):
                    self.lib.init_queue()
                self.use_c = True
                print(f"Loaded C backend from {LIB_PATH}")
            except Exception as e:
                print(f"Failed to load C library: {e}")
                self.lib = None
                self._init_python_backend()
        else:
            print("C library not found. Using Python fallback backend.")
            self._init_python_backend()

    def _setup_prototypes(self):
        # Define ctypes prototypes that we expect from the C library.
        # Update these if your C API differs.
        self.lib.init_queue.restype = c_int

        self.lib.add_process.argtypes = (c_int, c_int, c_int)
        self.lib.add_process.restype = c_int

        self.lib.delete_process.argtypes = (c_int,)
        self.lib.delete_process.restype = c_int

        self.lib.get_process_count.restype = c_int

        # get_process_list(int *pids, int *arrivals, int *bursts, int max)
        self.lib.get_process_list.argtypes = (POINTER(c_int), POINTER(c_int), POINTER(c_int), c_int)
        self.lib.get_process_list.restype = c_int

        # compute_stats(double* avg_wait, double* avg_turn)
        self.lib.compute_stats.argtypes = (POINTER(c_double), POINTER(c_double))
        self.lib.compute_stats.restype = c_int

    def _init_python_backend(self):
        # simple lists implementing FCFS behaviour
        self._queue = []  # list of dicts: {pid, arrival, burst}

    # Public API used by GUI code
    def add_process(self, pid: int, arrival: int, burst: int) -> bool:
        if self.use_c:
            res = self.lib.add_process(pid, arrival, burst)
            return res == 0
        # python fallback
        for p in self._queue:
            if p['pid'] == pid:
                return False
        self._queue.append({'pid': pid, 'arrival': arrival, 'burst': burst})
        # keep queue sorted by arrival for FCFS
        self._queue.sort(key=lambda x: x['arrival'])
        return True

    def delete_process(self, pid: int) -> bool:
        if self.use_c:
            res = self.lib.delete_process(pid)
            return res == 0
        for i, p in enumerate(self._queue):
            if p['pid'] == pid:
                self._queue.pop(i)
                return True
        return False

    def get_process_list(self):
        if self.use_c:
            count = self.lib.get_process_count()
            maxc = max(64, count)
            PIDS = (c_int * maxc)()
            ARRS = (c_int * maxc)()
            BURS = (c_int * maxc)()
            placed = self.lib.get_process_list(PIDS, ARRS, BURS, maxc)
            res = []
            for i in range(placed):
                res.append({'pid': int(PIDS[i]), 'arrival': int(ARRS[i]), 'burst': int(BURS[i])})
            return res
        return list(self._queue)  # copy

    def compute_stats(self):
        """Return (avg_wait, avg_turnaround, total_processes)
        Stats computed using FCFS on the current queue (fallback) or ask C lib.
        """
        if self.use_c:
            aw = c_double()
            at = c_double()
            res = self.lib.compute_stats(byref(aw), byref(at))
            if res == 0:
                total = self.lib.get_process_count()
                return (float(aw.value), float(at.value), int(total))
            return (0.0, 0.0, 0)

        q = sorted(self._queue, key=lambda x: x['arrival'])
        if not q:
            return (0.0, 0.0, 0)
        time_cursor = q[0]['arrival']
        total_wait = 0
        total_turn = 0
        for p in q:
            if time_cursor < p['arrival']:
                time_cursor = p['arrival']
            wait = time_cursor - p['arrival']
            total_wait += wait
            time_cursor += p['burst']
            turnaround = time_cursor - p['arrival']
            total_turn += turnaround
        n = len(q)
        return (total_wait / n, total_turn / n, n)


# ---------------------- GUI ----------------------
if TK_AVAILABLE:
    class ProcessSchedulerGUI(tk.Tk):
        def __init__(self, backend: BackendInterface):
            super().__init__()
            self.backend = backend
            self.title("Process Scheduler - GUI (Nishitha)")
            self.geometry('780x480')
            self.resizable(False, False)

            self._create_widgets()
            self._layout_widgets()
            self._start_clock()
            self.refresh_ui()

        def _create_widgets(self):
            # Inputs frame
            self.frm_inputs = ttk.LabelFrame(self, text="Process Controls")

            ttk.Label(self.frm_inputs, text="PID:").grid(row=0, column=0, sticky='e')
            self.ent_pid = ttk.Entry(self.frm_inputs, width=10)
            self.ent_pid.grid(row=0, column=1, padx=4, pady=4)

            ttk.Label(self.frm_inputs, text="Arrival:").grid(row=0, column=2, sticky='e')
            self.ent_arrival = ttk.Entry(self.frm_inputs, width=10)
            self.ent_arrival.grid(row=0, column=3, padx=4, pady=4)

            ttk.Label(self.frm_inputs, text="Burst:").grid(row=0, column=4, sticky='e')
            self.ent_burst = ttk.Entry(self.frm_inputs, width=10)
            self.ent_burst.grid(row=0, column=5, padx=4, pady=4)

            self.btn_add = ttk.Button(self.frm_inputs, text="Add Process", command=self._on_add)
            self.btn_add.grid(row=0, column=6, padx=6)

            ttk.Separator(self, orient='vertical')

            # Delete frame
            self.frm_delete = ttk.LabelFrame(self, text='Delete Process')
            ttk.Label(self.frm_delete, text='PID:').grid(row=0, column=0)
            self.ent_del_pid = ttk.Entry(self.frm_delete, width=10)
            self.ent_del_pid.grid(row=0, column=1, padx=4, pady=4)
            self.btn_del = ttk.Button(self.frm_delete, text='Delete Process', command=self._on_delete)
            self.btn_del.grid(row=0, column=2, padx=6)

            # Treeview for queue
            self.frm_table = ttk.LabelFrame(self, text='Ready Queue')
            cols = ('pid', 'arrival', 'burst')
            self.tree = ttk.Treeview(self.frm_table, columns=cols, show='headings', height=12)
            self.tree.heading('pid', text='PID')
            self.tree.heading('arrival', text='Arrival')
            self.tree.heading('burst', text='Burst')
            self.tree.column('pid', width=100, anchor='center')
            self.tree.column('arrival', width=120, anchor='center')
            self.tree.column('burst', width=120, anchor='center')

            self.scr = ttk.Scrollbar(self.frm_table, orient='vertical', command=self.tree.yview)
            self.tree.configure(yscrollcommand=self.scr.set)

            # Stats frame
            self.frm_stats = ttk.LabelFrame(self, text='Statistics')
            self.lbl_algo = ttk.Label(self.frm_stats, text='Algorithm: FCFS (First-Come-First-Serve)')
            self.lbl_total = ttk.Label(self.frm_stats, text='Total Processes: 0')
            self.lbl_avg_wait = ttk.Label(self.frm_stats, text='Average Waiting Time: 0.00')
            self.lbl_avg_turn = ttk.Label(self.frm_stats, text='Average Turnaround Time: 0.00')

            # Clock
            self.lbl_clock = ttk.Label(self, text='Simulation Time: --:--:--')

        def _layout_widgets(self):
            self.frm_inputs.place(x=10, y=10, width=760, height=60)
            self.frm_delete.place(x=10, y=70, width=760, height=60)
            self.frm_table.place(x=10, y=140, width=500, height=320)
            self.scr.place(in_=self.frm_table, relx=1.0, rely=0, relheight=1.0, x=-18)
            self.tree.place(x=10, y=10, width=470, height=280)

            self.frm_stats.place(x=520, y=140, width=250, height=200)
            self.lbl_algo.pack(anchor='w', padx=8, pady=8)
            self.lbl_total.pack(anchor='w', padx=8, pady=4)
            self.lbl_avg_wait.pack(anchor='w', padx=8, pady=4)
            self.lbl_avg_turn.pack(anchor='w', padx=8, pady=4)

            self.lbl_clock.place(x=520, y=360)

        # ----------------- Event handlers -----------------
        def _on_add(self):
            try:
                pid = int(self.ent_pid.get().strip())
                arrival = int(self.ent_arrival.get().strip())
                burst = int(self.ent_burst.get().strip())
            except ValueError:
                messagebox.showerror("Invalid input", "PID, Arrival and Burst must be integers")
                return
            ok = self.backend.add_process(pid, arrival, burst)
            if not ok:
                messagebox.showwarning("Add failed", f"Process with PID {pid} already exists or C call failed")
                return
            self._clear_input_fields()
            self.refresh_ui()

        def _on_delete(self):
            try:
                pid = int(self.ent_del_pid.get().strip())
            except ValueError:
                messagebox.showerror("Invalid input", "PID must be integer")
                return
            ok = self.backend.delete_process(pid)
            if not ok:
                messagebox.showwarning("Delete failed", f"Process with PID {pid} not found or C call failed")
                return
            self.ent_del_pid.delete(0, tk.END)
            self.refresh_ui()

        def _clear_input_fields(self):
            self.ent_pid.delete(0, tk.END)
            self.ent_arrival.delete(0, tk.END)
            self.ent_burst.delete(0, tk.END)

        # ----------------- UI Refresh -----------------
        def refresh_ui(self):
            # Update tree
            for i in self.tree.get_children():
                self.tree.delete(i)
            plist = self.backend.get_process_list()
            for p in plist:
                self.tree.insert('', 'end', values=(p['pid'], p['arrival'], p['burst']))

            # Update stats
            aw, at, tot = self.backend.compute_stats()
            self.lbl_total.config(text=f"Total Processes: {tot}")
            self.lbl_avg_wait.config(text=f"Average Waiting Time: {aw:.2f}")
            self.lbl_avg_turn.config(text=f"Average Turnaround Time: {at:.2f}")

        # ----------------- Clock -----------------
        def _start_clock(self):
            def tick():
                now = time.strftime('%H:%M:%S')
                self.lbl_clock.config(text=f"Simulation Time: {now}")
                self.after(1000, tick)
            tick()


# ---------------------- CLI / Demo (used when tkinter unavailable) ----------------------
else:
    def cli_show_queue(backend: BackendInterface):
        plist = backend.get_process_list()
        if not plist:
            print("Ready Queue: <empty>")
            return
        print("Ready Queue:")
        print(f"{'PID':>6} {'Arrival':>8} {'Burst':>6}")
        for p in plist:
            print(f"{p['pid']:6d} {p['arrival']:8d} {p['burst']:6d}")

    def cli_show_stats(backend: BackendInterface):
        aw, at, tot = backend.compute_stats()
        print(f"Algorithm: FCFS (fallback)")
        print(f"Total Processes: {tot}")
        print(f"Average Waiting Time: {aw:.2f}")
        print(f"Average Turnaround Time: {at:.2f}")

    def cli_demo_sequence(backend: BackendInterface):
        print("Running CLI demo sequence (adds/deletes several processes and prints stats)...\n")
        tests = [
            (101, 0, 5),
            (102, 2, 3),
            (103, 4, 1),
            (104, 6, 7),
            (105, 6, 2),  # same arrival as 104 to test tie behaviour (sorted by insertion order after stable sort)
        ]
        # Add processes
        for pid, arr, burst in tests:
            ok = backend.add_process(pid, arr, burst)
            print(f"add_process(pid={pid}, arrival={arr}, burst={burst}) -> {ok}")
        print()
        cli_show_queue(backend)
        print()
        cli_show_stats(backend)
        print("\nDeleting PID 103 and attempting to delete missing PID 999...\n")
        d1 = backend.delete_process(103)
        d2 = backend.delete_process(999)
        print(f"delete_process(103) -> {d1}")
        print(f"delete_process(999) -> {d2}")
        print()
        cli_show_queue(backend)
        print()
        cli_show_stats(backend)

    def cli_interactive(backend: BackendInterface):
        print("Interactive CLI mode. Commands: add PID ARR BURST | del PID | show | stats | exit")
        while True:
            try:
                line = input('> ').strip()
            except (EOFError, KeyboardInterrupt):
                print('\nExiting interactive CLI.')
                break
            if not line:
                continue
            parts = line.split()
            cmd = parts[0].lower()
            if cmd == 'add' and len(parts) == 4:
                try:
                    pid = int(parts[1]); arr = int(parts[2]); burst = int(parts[3])
                except ValueError:
                    print('Invalid integers.')
                    continue
                print('OK' if backend.add_process(pid, arr, burst) else 'FAILED')
            elif cmd == 'del' and len(parts) == 2:
                try:
                    pid = int(parts[1])
                except ValueError:
                    print('Invalid PID')
                    continue
                print('OK' if backend.delete_process(pid) else 'NOT FOUND')
            elif cmd == 'show':
                cli_show_queue(backend)
            elif cmd == 'stats':
                cli_show_stats(backend)
            elif cmd == 'exit':
                break
            else:
                print('Unknown command or wrong args.')


# ---------------------- Main ----------------------

def main():
    backend = BackendInterface()
    if TK_AVAILABLE:
        app = __import__('__main__').ProcessSchedulerGUI(backend)  # reference class defined above
        app.mainloop()
    else:
        # Run some automated tests first (always add test cases)
        cli_demo_sequence(backend)
        # Then offer interactive CLI if running in an interactive terminal
        if sys.stdin.isatty():
            cli_interactive(backend)


if __name__ == '__main__':
    main()
