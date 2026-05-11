#define _DEFAULT_SOURCE // Enable extensions like d_type in dirent.h
                        // Must be defined before any includes

#include <stdio.h>
#include <dirent.h> // For directory operations (opendir, readdir, closedir)
#include <unistd.h> // Provides access to POSIX OS API
#include <time.h>   // For time functions (time, localtime, strftime)
#include <string.h> // For string manipulation (memset, strcmp, strncmp, strcpy)
#include <stdlib.h> // For general utilities (atoi, qsort, exit)
#include <ctype.h>  // For character type checking (isdigit)
#include <errno.h>  // For error number variables (perror)

#define MAX_PROC_NUM 1024
#define MAX_LINE_LEN 256 // Max length for reading lines from /proc files

typedef struct _my_process
{
    int pid;
    char pname[64]; // Process name
    char pstate;    // Process state (single character like 'R', 'S')
    int ppid;       // Parent PID
    int memsize;    // Virtual memory size in KB (from VmSize)
} my_process;

my_process procs[MAX_PROC_NUM];
int proc_count = 0; // Keep track of the number of processes found

// Comparison function for qsort to sort processes by memory size descending
int compare_proc_mem(const void *a, const void *b)
{
    my_process *proc_a = (my_process *)a;
    my_process *proc_b = (my_process *)b;
    // Sort descending: return positive if b > a
    return proc_b->memsize - proc_a->memsize;
}

// Outputs current time
void output_time()
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);              // Get current time
    timeinfo = localtime(&rawtime); // Convert to local time structure

    // Format time as<y_bin_46>-MM-DD HH:MM:SS
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    printf("Time: %s\n", buffer);
}

// Outputs system uptime
void output_uptime()
{
    FILE *fp = fopen("/proc/uptime", "r");
    if (fp == NULL)
    {
        perror("Failed to open /proc/uptime");
        return;
    }

    double uptime_seconds;
    if (fscanf(fp, "%lf", &uptime_seconds) == 1)
    {
        int days = (int)uptime_seconds / (24 * 3600);
        int hours = ((int)uptime_seconds % (24 * 3600)) / 3600;
        int minutes = ((int)uptime_seconds % 3600) / 60;
        // int seconds = (int)uptime_seconds % 60; // Not shown in sample
        printf("Uptime: %d days, %02d:%02d\n", days, hours, minutes);
    }
    else
    {
        fprintf(stderr, "Failed to read uptime from /proc/uptime\n");
    }
    fclose(fp);
}

// Outputs load average
void output_loadavg()
{
    FILE *fp = fopen("/proc/loadavg", "r");
    if (fp == NULL)
    { 
        perror("Failed to open /proc/loadavg");
        return;
    }

    double load1, load5, load15;
    // Read the first three floating point numbers
    if (fscanf(fp, "%lf %lf %lf", &load1, &load5, &load15) == 3)
    {
        printf("Load avg: %f %f %f\n", load1, load5, load15);
    }
    else
    {
        fprintf(stderr, "Failed to read load average from /proc/loadavg\n");
    }
    fclose(fp);
}

// Outputs CPU statistics (from /proc/stat)
void output_cpu()
{
    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        perror("Failed to open /proc/stat");
        return;
    }

    char line[MAX_LINE_LEN];
    long long user, nice, system, idle, iowait, irq, softirq, steal; // Use long long for potentially large jiffies counts

    while (fgets(line, sizeof(line), fp))
    {
        // Find the line starting with "cpu " (note the space)
        if (strncmp(line, "cpu ", 4) == 0)
        {
            // Parse the values from the cpu line
            int matched = sscanf(line, "cpu %lld %lld %lld %lld %lld %lld %lld %lld",
                   &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
            // The sample output format seems to map to these 8 values
            if (matched >= 8)
            {
                 // us = user, sy = system, ni = nice, id = idle, wa = iowait, hi = irq, si = softirq, st = steal
                printf("Cpu: %lldus %lldsy %lldni %lldid %lldwa %lldhi %lldsi %lldst\n",
                       user, system, nice, idle, iowait, irq, softirq, steal);
            } else {
                 fprintf(stderr, "Failed to parse cpu line in /proc/stat correctly (matched %d)\n", matched);
            }
            break; // Found the line, no need to read further
        }
    }
    fclose(fp);
}


// Outputs memory information (from /proc/meminfo)
void output_mem()
{
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
    {
        perror("Failed to open /proc/meminfo");
        return;
    }

    char line[MAX_LINE_LEN];
    // Removed unused mem_available
    long mem_total = -1, mem_free = -1; // Use long, units are KB

    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "MemTotal:", 9) == 0)
        {
            sscanf(line, "MemTotal: %ld kB", &mem_total);
        }
        else if (strncmp(line, "MemFree:", 8) == 0)
        {
            sscanf(line, "MemFree: %ld kB", &mem_free);
        }
        // Removed MemAvailable parsing

        // Break early if we found what we need
        if (mem_total != -1 && mem_free != -1)
        {
            break;
        }
    }
    fclose(fp);

    if (mem_total != -1 && mem_free != -1)
    {
        long mem_used = mem_total - mem_free; // Calculate used memory
        printf("Mem: %ld total %ld used %ld free\n", mem_total, mem_used, mem_free);
    }
    else
    {
        fprintf(stderr, "Failed to read memory info from /proc/meminfo\n");
    }
}

// Outputs swap information (from /proc/meminfo)
void output_swap()
{
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
    {
        perror("Failed to open /proc/meminfo");
        return;
    }

    char line[MAX_LINE_LEN];
    long swap_total = -1, swap_free = -1; // Use long, units are KB

    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "SwapTotal:", 10) == 0)
        {
            sscanf(line, "SwapTotal: %ld kB", &swap_total);
        }
        else if (strncmp(line, "SwapFree:", 9) == 0)
        {
            sscanf(line, "SwapFree: %ld kB", &swap_free);
        }

        // Break early if we found what we need
        if (swap_total != -1 && swap_free != -1)
        {
            break;
        }
    }
    fclose(fp);

    if (swap_total != -1 && swap_free != -1)
    {
         // Avoid division by zero or negative results if swap isn't configured
        if (swap_total > 0) {
            long swap_used = swap_total - swap_free;
            printf("Swap: %ld total %ld used %ld free\n", swap_total, swap_used, swap_free);
        } else {
             printf("Swap: 0 total 0 used 0 free\n"); // No swap configured
        }
    }
    else
    {
        // If SwapTotal or SwapFree weren't found, assume no swap or error reading
        fprintf(stderr, "Failed to read full swap info from /proc/meminfo, reporting 0\n");
        printf("Swap: 0 total 0 used 0 free\n");
    }
}

// Checks if a string consists only of digits
int is_numeric(const char *s) {
    if (s == NULL || *s == '\0') return 0; // Empty string is not numeric
    while (*s) {
        if (!isdigit(*s)) return 0; // False if any non-digit found
        s++;
    }
    return 1; // True if all chars are digits
}


// Gets all current running processes
void getAllProcess()
{
    DIR *dir = opendir("/proc");
    if (dir == NULL)
    {
        perror("Failed to open /proc");
        exit(EXIT_FAILURE); // Cannot continue without /proc
    }

    struct dirent *entry;
    proc_count = 0; // Reset process count

    // Iterate through entries in /proc
    while ((entry = readdir(dir)) != NULL && proc_count < MAX_PROC_NUM)
    {
        // Check if the directory name is a number (process ID)
        // Use d_type if available (enabled by _DEFAULT_SOURCE) for efficiency
        if (entry->d_type == DT_DIR && is_numeric(entry->d_name))
        {
            char status_path[256];
            snprintf(status_path, sizeof(status_path), "/proc/%s/status", entry->d_name);

            FILE *fp = fopen(status_path, "r");
            // Process might disappear between readdir and fopen
            if (fp == NULL) {
                // Optionally print warning: perror(status_path);
                continue;
            }

            char line[MAX_LINE_LEN];
            my_process current_proc = {0}; // Initialize current process struct
            current_proc.pid = atoi(entry->d_name); // PID is the directory name

            int found_name = 0, found_state = 0, found_ppid = 0, found_vmsize = 0;

            // Read the status file line by line
            while (fgets(line, sizeof(line), fp))
            {
                if (strncmp(line, "Name:", 5) == 0)
                {
                    sscanf(line, "Name:\t%63s", current_proc.pname); // Read process name
                    found_name = 1;
                }
                else if (strncmp(line, "State:", 6) == 0)
                {
                    // Removed unused state_desc
                    // State line looks like: State: S (sleeping)
                    sscanf(line, "State:\t%c", &current_proc.pstate); // Read just the state character
                    found_state = 1;
                }
                else if (strncmp(line, "PPid:", 5) == 0)
                {
                    sscanf(line, "PPid:\t%d", &current_proc.ppid); // Read parent PID
                    found_ppid = 1;
                }
                else if (strncmp(line, "VmSize:", 7) == 0)
                {
                    // VmSize is Virtual Memory Size, usually in kB
                    sscanf(line, "VmSize:\t%d kB", &current_proc.memsize);
                    found_vmsize = 1;
                }
                // Optimization: Stop reading if all needed fields are found
                if (found_name && found_state && found_ppid && found_vmsize) {
                    break;
                }
            }
            fclose(fp);

            // Only add the process if we found the necessary info (especially memsize)
            // Ignore processes with 0 memsize as per hints
            if (found_name && found_state && found_ppid && found_vmsize && current_proc.memsize > 0) {
                procs[proc_count++] = current_proc;
            } else {
                // Optionally print debug message if a process entry was incomplete
                // fprintf(stderr, "Skipping incomplete process %d\n", current_proc.pid);
            }
        }
        // Alternative check if d_type is not available or reliable (slower):
        /*
        else if (is_numeric(entry->d_name)) { // It's a PID, but d_type wasn't DT_DIR or is unavailable
            struct stat st;
            char path[256];
            snprintf(path, sizeof(path), "/proc/%s", entry->d_name);
            if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
                 // It's confirmed a directory, proceed with reading status...
                 // Code from inside the 'if (entry->d_type == DT_DIR ...)' block would go here
            }
        }
        */
    }
    closedir(dir);
}

// Outputs process statistics based on collected data
void output_procstat()
{
    int running = 0;
    int sleeping = 0;
    // Other states like Zombie (Z), Stopped (T), Paging (W), Dead (X), etc. exist
    // but sample only shows running and sleeping.

    for (int i = 0; i < proc_count; i++)
    {
        switch (procs[i].pstate)
        {
        case 'R':
            running++;
            break;
        case 'S':
            sleeping++;
            break;
        // Add cases for other states if needed
        }
    }
    // Total count is proc_count
    printf("Tasks: %d total %d running %d sleeping\n", proc_count, running, sleeping);
}

// Outputs top 10 processes that have the largest memory sizes in descending order
void output_proc()
{
    // Sort the collected processes by memory size (descending)
    qsort(procs, proc_count, sizeof(my_process), compare_proc_mem);

    // Determine how many processes to print (max 10 or fewer if less exist)
    int count_to_print = (proc_count < 10) ? proc_count : 10;

    // Print the header implicitly via the first line's format, or add explicitly:
    // printf("%5s %-15s %s %5s %10s\n", "PID", "(NAME)", "S", "PPID", "MEM(KB)");

    // Print the top processes
    for (int i = 0; i < count_to_print; i++)
    {
        // Format matches the sample output: PID (NAME) STATE PPID MEMSIZE
        printf("%-7d (%-15s) %c %7d %10d\n",
               procs[i].pid,
               procs[i].pname,
               procs[i].pstate,
               procs[i].ppid,
               procs[i].memsize);
    }
}

int main(int argc, char *argv[])
{
    // No need to memset procs globally declared, they start at 0.
    // But if it were local, memset would be good.
    // memset(&procs, 0, MAX_PROC_NUM * sizeof(my_process));

    getAllProcess(); // Collect all process data first

    // Output system information
    output_time();
    output_uptime();
    output_loadavg();
    output_procstat(); // Needs process data collected by getAllProcess
    output_cpu();
    output_mem();
    output_swap();

    // Output top processes (needs sorted data from getAllProcess)
    output_proc();

    return 0;
}