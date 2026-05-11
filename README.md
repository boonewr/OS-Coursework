# CSC 246 Operating Systems Coursework

This repository contains completed problem sets and exercises from CSC 246, a university operating systems course. The work is mostly C on Linux/POSIX APIs, with later assignments using CUDA, TCP sockets, `/proc`, and small GUI/application packaging tools.

Compiled binaries, object files, local editor settings, and generated build directories are intentionally omitted. The repository keeps source code and the input/expected-output data that make the coursework reviewable.

## Topics Covered

- Process creation and control with `fork()`, `exec()`, `wait()`, pipes, and redirection
- POSIX message queues, shared memory, semaphores, mutexes, and condition variables
- Thread coordination, monitor-style synchronization, deadlock avoidance, and critical sections
- Memory management with `mmap()`, a custom allocator, and binary file mapping
- Networked programs using TCP sockets, a simple HTTP client, and ONC/RPC
- File-system parsing through FAT12 disk-image directory traversal
- Linux system introspection via `/proc`
- CUDA-based parallel computation

## Repository Layout

```text
assignments/   Larger homework projects, grouped by homework number.
exercises/     Shorter lab-style exercises, grouped by exercise number.
```

Most programs were written for a Linux environment. Typical builds use `gcc`, sometimes with `-pthread`, `-lrt`, or `-lm` depending on the assignment. CUDA work requires `nvcc`; RPC work requires `rpcgen`; the Java test server in `exercises/ex16` requires a JDK.

## Assignments

### Homework 1: System Calls and a Small Shell

`assignments/hw1/stash.c` implements a simple command shell. It parses command lines, supports built-in `cd` and `exit`, launches external commands with `fork()` and `execvp()`, waits for foreground commands, and tracks a background child with non-blocking `waitpid()`.

`assignments/hw1/exclude.c` is a file-processing utility built directly on Unix system calls. It opens an input and output file, reads bytes with `read()`, writes with `write()`, and omits a selected 1-based line number.

### Homework 2: Processes, Pipes, and POSIX Message Queues

`assignments/hw2/triangles.c` counts all valid triangles that can be formed from an input list of side lengths. It divides the search across worker processes, sends each worker's count back through a pipe, and uses `lockf()` around pipe writes.

`assignments/hw2/server.c`, `client.c`, and `common.h` implement a string-editing client/server pair using POSIX message queues. Clients can insert, delete, undo, and report state; the server owns the editable string and handles shutdown with a signal handler.

`interleavings.txt` records scheduling/interleaving reasoning for concurrent execution.

### Homework 3: Pthreads and Shared Memory

`assignments/hw3/triangles-thread.c` revisits the triangle-counting problem with POSIX threads instead of worker processes. Each thread receives a worker id, searches a strided partition of the combination space, and reports a local count that is joined into the final total.

`reset.c`, `edit.c`, `common.h`, and the alternate `*2` files implement a shared-memory string editor. `reset` initializes the shared state, while `edit` attaches to it and performs insert, delete, undo, and report operations.

### Homework 4: Semaphores and Safer Shared State

`assignments/hw4/triangles-sem.c` adds semaphore-based work distribution to the threaded triangle counter. Workers acquire work from a shared index, coordinate through semaphores, and protect the global count with a mutex.

`reset.c`, `edit.c`, and `common.h` extend the shared-memory editor with a named semaphore for mutual exclusion. `hw4_p1.txt` contains synchronization/interleaving analysis.

### Homework 5: Monitors and Deadlock Avoidance

`assignments/hw5/matchmaker.c` and `matchmaker.h` implement a monitor for pairing player threads by compatible skill level. It uses a mutex, condition variables, a waiting list, and a shutdown path to coordinate matches and completion.

The driver files exercise different timing and pairing scenarios. The kitchen simulation files compare synchronization strategies around shared appliances: coarse-grained global locking, ordered locking, and acquiring all needed appliances. `kitchen.txt` summarizes observed throughput across those strategies.

### Homework 6: CUDA and a Multithreaded TCP Server

`assignments/hw6/triangles.cu` ports the triangle-counting workload to CUDA. It copies the input list to device memory, launches a kernel where each CUDA thread counts triangles ending at its index, copies per-thread results back, and reduces them on the host.

`assignments/hw6/parkingServer.c` is a concurrent TCP parking-lot server. Each client connection is handled on a detached pthread, and a mutex protects the shared parking table while commands park vehicles, calculate leaving fees, show active records, and quit sessions.

### Homework 7: Linux Process Inspection and Application Packaging

`assignments/hw7/top.c` implements a small `top`-style Linux system report by reading `/proc`. It reports current time, uptime, load average, task states, CPU counters, memory/swap totals, and the top processes by virtual memory size.

`topO.c` appears to be the provided skeleton, while `top 2.c` is an alternate/annotated implementation. `t1.py` is a small Tkinter text editor with New/Open/Save/Exit menu actions, with `t1.spec` capturing PyInstaller packaging metadata.

## Exercises

The exercises are shorter focused programs:

- `ex1` through `ex4`: system-call overhead, child creation, shell-style pipelines, and output redirection.
- `ex5` through `ex9`: pthread scheduling, semaphore ping-pong coordination, critical sections, and deadlock behavior.
- `ex11`: a custom `malloc`, `free`, `calloc`, and `realloc` implementation backed by `mmap()` and a coalescing free list.
- `ex14`: binary point-of-interest sorting with `mmap()` and great-circle distance calculations.
- `ex15`: array-backed and linked-list sorting implementations for comparing memory/layout tradeoffs.
- `ex16`: a C HTTP client tested against a small Java HTTP server.
- `ex17`: an ONC/RPC message-of-the-day service with generated RPC support files and a test script.
- `ex18`: FAT12 floppy disk-image directory readers, including recursive directory traversal and expected outputs.

## Notes

This repository is a course archive. Some programs assume the original course Linux environment, usernames, queue names, or local paths in `ftok()` calls. Those details are left intact to preserve the submitted coursework, but they may need adjustment before rebuilding on a different machine.
