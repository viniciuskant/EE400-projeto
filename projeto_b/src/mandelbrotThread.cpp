#include <cinttypes>
#include <thread>
#include <cstdio>  
#include <cstdlib>  



typedef struct {
    long double x0, x1;
    long double y0, y1;
    unsigned int width;
    unsigned int height;
    int maxIterations;
    int* output;
    int threadId;
    int startRow;
    int numRows;
} WorkerArgs;


extern void mandelbrotSerial(
    long double x0, long double y0, long double x1, long double y1,
    int width, int height,
    int startRow, int numRows,
    int maxIterations,
    int output[]);

void workerThreadStart(WorkerArgs * const args) {
    mandelbrotSerial(
        static_cast<long double>(args->x0), static_cast<long double>(args->y0), 
        static_cast<long double>(args->x1), static_cast<long double>(args->y1),
        args->width, args->height,
        args->startRow, 
        args->numRows,
        args->maxIterations,
        args->output
    );
}

void mandelbrotThread(
    int numThreads,
    long double x0, long double y0, long double x1, long double y1,
    int width, int height,
    int maxIterations, int output[])
{
    static constexpr int MAX_THREADS = 32;

    if (numThreads > MAX_THREADS) {
        fprintf(stderr, "Error: Max allowed threads is %d\n", MAX_THREADS);
        exit(1);
    }

    std::thread workers[MAX_THREADS];
    WorkerArgs args[MAX_THREADS];

    int rowsPerThread = height / numThreads;
    int remainingRows = height % numThreads;

    int currentRow = 0;

    for (int i = 0; i < numThreads; i++) {
        int tmp_height = rowsPerThread + (i == numThreads - 1 ? remainingRows : 0);

        args[i].x0 = x0;
        args[i].x1 = x1;
        args[i].y0 = y0;
        args[i].y1 = y1;
        args[i].width = width;
        args[i].height = height;
        args[i].numRows = tmp_height;
        args[i].maxIterations = maxIterations;
        args[i].output = output;
        args[i].threadId = i;
        args[i].startRow = currentRow;

        currentRow += tmp_height;
    }

    for (int i = 1; i < numThreads; i++) {
        workers[i] = std::thread(workerThreadStart, &args[i]);
    }

    workerThreadStart(&args[0]);

    for (int i = 1; i < numThreads; i++) {
        workers[i].join();
    }
}