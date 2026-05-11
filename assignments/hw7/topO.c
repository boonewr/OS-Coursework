#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define	MAX_PROC_NUM	1024

typedef struct _my_process
{
	int pid;
	char pname[64];
	char pstate[8];
	int ppid;
	int memsize;
}my_process;

my_process procs[MAX_PROC_NUM];

// Outputs current time
void output_time()
{	
}

// Outputs system uptime
void output_uptime()
{	
}

// Outputs load average
void output_loadavg()
{
}

// Outputs CPU
void output_cpu()
{
}

// Outputs memory information
void output_mem()
{
}

// Outputs swap information
void output_swap()
{
}

// Gets all current running processes 
void getAllProcess()
{
}

// Outputs process statistics
void output_procstat()
{
}

// Outputs top 10 processes that have the largest memory sizes in descending order
void output_proc()
{
}

int main(int argc, char* argv[])
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

