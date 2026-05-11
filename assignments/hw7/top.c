#define _DEFAULT_SOURCE

#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#define MAX_PROC_NUM 1024
#define MAX_LINE_LEN 256

typedef struct _my_process
{
    int pid;
    char pname[64];
    char pstate;
    int ppid;
    int memsize;
} my_process;

my_process procs[MAX_PROC_NUM];
int proc_count = 0;

int compare_proc_mem(const void *a, const void *b)
{
    my_process *proc_a = (my_process *)a;
    my_process *proc_b = (my_process *)b;
    return proc_b->memsize - proc_a->memsize;
}

// Outputs current time
void output_time()
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

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
        printf("Uptime: %d days, %02d:%02d\n", days, hours, minutes);
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
    if (fscanf(fp, "%lf %lf %lf", &load1, &load5, &load15) == 3)
    {
        printf("Load avg: %f %f %f\n", load1, load5, load15);
    }
    else
    {
        fprintf(stderr, "Failed to read load average\n");
    }
    fclose(fp);
}

// Outputs CPU
void output_cpu()
{
    FILE *fp = fopen("/proc/stat", "r");

    char line[MAX_LINE_LEN];
    long long user, nice, system, idle, iowait, irq, softirq, steal;

    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "cpu ", 4) == 0)
        {
            int matched = sscanf(line, "cpu %lld %lld %lld %lld %lld %lld %lld %lld",
                                 &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
            if (matched >= 8)
            {
                printf("Cpu: %lldus %lldsy %lldni %lldid %lldwa %lldhi %lldsi %lldst\n",
                       user, system, nice, idle, iowait, irq, softirq, steal);
            }
            else
            {
                fprintf(stderr, "Failed to parse cpu\n");
            }
            break;
        }
    }
    fclose(fp);
}

// Outputs memory information
void output_mem()
{
    FILE *fp = fopen("/proc/meminfo", "r");

    char line[MAX_LINE_LEN];
    long mem_total = -1, mem_free = -1;

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
        if (mem_total != -1 && mem_free != -1)
        {
            break;
        }
    }
    fclose(fp);

    if (mem_total != -1 && mem_free != -1)
    {
        long mem_used = mem_total - mem_free;
        printf("Mem: %ld total %ld used %ld free\n", mem_total, mem_used, mem_free);
    }
}

// Outputs swap information
void output_swap()
{
    FILE *fp = fopen("/proc/meminfo", "r");

    char line[MAX_LINE_LEN];
    long swap_total = -1, swap_free = -1;

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

        if (swap_total != -1 && swap_free != -1)
        {
            break;
        }
    }
    fclose(fp);

    if (swap_total != -1 && swap_free != -1)
    {
        if (swap_total > 0)
        {
            long swap_used = swap_total - swap_free;
            printf("Swap: %ld total %ld used %ld free\n", swap_total, swap_used, swap_free);
        }
        else
        {
            printf("Swap: 0 total 0 used 0 free\n");
        }
    }
    else
    {
        fprintf(stderr, "Failed to read full swap info\n");
        printf("Swap: 0 total 0 used 0 free\n");
    }
}

// Checks if a string is a number
int is_numeric(const char *s)
{
    if (s == NULL || *s == '\0')
        return 0;
    while (*s)
    {
        if (!isdigit(*s))
            return 0;
        s++;
    }
    return 1;
}

// Gets all current running processes
void getAllProcess()
{
    DIR *dir = opendir("/proc");
    if (dir == NULL)
    {
        perror("Failed to open proc");
        exit(EXIT_FAILURE);
    }

    struct dirent *entry;
    proc_count = 0;

    while ((entry = readdir(dir)) != NULL && proc_count < MAX_PROC_NUM)
    {
        if (entry->d_type == DT_DIR && is_numeric(entry->d_name))
        {
            char status_path[512];
            snprintf(status_path, sizeof(status_path), "/proc/%s/status", entry->d_name);

            FILE *fp = fopen(status_path, "r");
            if (fp == NULL)
            {
                continue;
            }

            char line[MAX_LINE_LEN];
            my_process current_proc = {0};
            current_proc.pid = atoi(entry->d_name);

            int found_name = 0, found_state = 0, found_ppid = 0, found_vmsize = 0;

            while (fgets(line, sizeof(line), fp))
            {
                if (strncmp(line, "Name:", 5) == 0)
                {
                    sscanf(line, "Name:\t%63s", current_proc.pname);
                    found_name = 1;
                }
                else if (strncmp(line, "State:", 6) == 0)
                {
                    sscanf(line, "State:\t%c", &current_proc.pstate);
                    found_state = 1;
                }
                else if (strncmp(line, "PPid:", 5) == 0)
                {
                    sscanf(line, "PPid:\t%d", &current_proc.ppid);
                    found_ppid = 1;
                }
                else if (strncmp(line, "VmSize:", 7) == 0)
                {
                    sscanf(line, "VmSize:\t%d kB", &current_proc.memsize);
                    found_vmsize = 1;
                }
                if (found_name && found_state && found_ppid && found_vmsize)
                {
                    break;
                }
            }
            fclose(fp);

            if (found_name && found_state && found_ppid && found_vmsize && current_proc.memsize > 0)
            {
                procs[proc_count++] = current_proc;
            }
        }
    }
    closedir(dir);
}

// Outputs process statistics
void output_procstat()
{
    int running = 0;
    int sleeping = 0;
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
        }
    }
    printf("Tasks: %d total %d running %d sleeping\n", proc_count, running, sleeping);
}

// Outputs top 10 processes that have the largest memory sizes in descending order
void output_proc()
{
    qsort(procs, proc_count, sizeof(my_process), compare_proc_mem);

    int count_to_print = (proc_count < 10) ? proc_count : 10;

    for (int i = 0; i < count_to_print; i++)
    {
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
    memset(&procs, 0, MAX_PROC_NUM * sizeof(my_process));
    getAllProcess();
    output_time();
    output_uptime();
    output_loadavg();
    output_procstat();
    output_cpu();
    output_mem();
    output_swap();
    output_proc();
    return 0;
}