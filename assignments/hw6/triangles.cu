// Elapsed Real Time for input-4.txt:
// 0m0.393s

// Full output:
/*
Total count: 655612387

real    0m0.393s
user    0m0.016s
sys     0m0.088s
*/

/*
Note: Since we had the alternatives section at the end of the assignment docummentation, I completed this
locally with the CUDA compiler through Windows Subsystem for Linux (WSL).

All I needed for the install was to run the following in the WSL terminal:

$ sudo apt install nvidia-cuda-toolkit
$ sudo apt install g++

and then I was able to compile and run normally:

$ nvcc triangles.cu -o triangles
*/

#include <stdio.h>
#include <stdbool.h>
#include <cuda_runtime.h>

// Input sequence of integers.
int *vList;

// Number of integers on the list.
int vCount = 0;

// Capacity of the list of integers.
int vCap = 0;

// General function to report a failure and exit.
static void fail(char const *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}

// Print out a usage message, then exit.
static void usage()
{
    printf("usage: triangles [report]\n");
    exit(1);
}

// Read the list of positive integers.
__host__ void readList()
{
    // Set up initial list and capacity.
    vCap = 5;
    vList = (int *)malloc(vCap * sizeof(int));

    // Keep reading as many values as we can.
    int v;
    while (scanf("%d\n", &v) == 1)
    {
        // Grow the list if needed.
        if (vCount >= vCap)
        {
            vCap *= 2;
            vList = (int *)realloc(vList, vCap * sizeof(int));
        }

        // Store the latest value in the next array slot.
        vList[vCount++] = v;
    }
}

__global__ void checkValid(int vCount, int *vList, int *results, bool report)
{
    int idx = blockDim.x * blockIdx.x + threadIdx.x;

    if (idx >= vCount)
        return;

    int localCount = 0;

    // count valid triangles
    for (int i = 0; i < idx - 1; i++)
    {
        for (int j = i + 1; j < idx; j++)
        {
            int a = vList[i];
            int b = vList[j];
            int c = vList[idx];

            if (a + b > c && a + c > b && b + c > a)
            {
                localCount++;
            }
        }
    }

    // Separate out reporting
    if (report)
    {
        if (localCount == 0)
        {
            printf("I’m thread %d. Local count: %d.\n", idx, localCount);
        }
        else
        {

            for (int i = 0; i < idx - 1; i++)
            {
                for (int j = i + 1; j < idx; j++)
                {
                    int a = vList[i];
                    int b = vList[j];
                    int c = vList[idx];

                    if (a + b > c && a + c > b && b + c > a)
                    {
                        printf("I’m thread %d. Triangle (%d, %d, %d) found at: %d, %d and %d.\n",
                               idx, a, b, c, i, j, idx);
                    }
                }
            }
        }
    }

    results[idx] = localCount;
}

int main(int argc, char *argv[])
{
    if (argc < 1 || argc > 2)
        usage();

    // If there's an argument, it better be "report"
    bool report = false;
    if (argc == 2)
    {
        if (strcmp(argv[1], "report") != 0)
            usage();
        report = true;
    }

    readList();

    int *dev_vList;
    int *dev_results;

    // Allocate device memory for vList and results
    cudaMalloc(&dev_vList, vCount * sizeof(int));
    cudaMalloc(&dev_results, vCount * sizeof(int));

    // Add code to allocate memory on the device and copy over the list.
    cudaMemcpy(dev_vList, vList, vCount * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemset(dev_results, 0, vCount * sizeof(int));

    // Block and grid dimensions.
    int threadsPerBlock = 100;
    // Round up for the number of blocks we need.
    int blocksPerGrid = (vCount + threadsPerBlock - 1) / threadsPerBlock;

    // Run our kernel on these block/grid dimensions
    checkValid<<<blocksPerGrid, threadsPerBlock>>>(vCount, dev_vList, dev_results, report);
    if (cudaGetLastError() != cudaSuccess)
        fail("Failure in CUDA kernel execution.");

    // Add code to copy results back to the host, add up all the per-thread counts
    // and report one global total count.
    int *host_results = (int *)malloc(vCount * sizeof(int));
    cudaMemcpy(host_results, dev_results, vCount * sizeof(int), cudaMemcpyDeviceToHost);

    // Sum results
    int total = 0;
    for (int i = 0; i < vCount; i++)
    {
        total += host_results[i];
    }

    printf("Total count: %d\n", total);

    // Free memory on the device and the host.
    cudaFree(dev_vList);
    cudaFree(dev_results);
    free(host_results);
    free(vList);

    cudaDeviceReset();

    return 0;
}
