#include <cinttypes>
#include <thread>
#include <cstdio>
#include <cstdlib>

typedef struct {
    long double x0, x1;
    long double y0, y1;
    long double c_re, c_im;
    int e;
    unsigned int width;
    unsigned int height;
    int maxIterations;
    int* output;
    int threadId;
    int startRow;
    int numRows;
} JuliaWorkerArgs;

extern void juliaSerial(
    long double x0, long double y0, long double x1, long double y1,
    long double c_re, long double c_im,
    int e,
    int width, int height,
    int startRow, int totalRows,
    int maxIterations,
    int output[]);

void juliaWorkerThreadStart(JuliaWorkerArgs* const args) {
    juliaSerial(
        args->x0, args->y0,
        args->x1, args->y1,
        args->c_re, args->c_im,
        args->e,
        args->width, args->height,
        args->startRow,
        args->numRows,
        args->maxIterations,
        args->output
    );
}

void juliaThread(
    int numThreads,
    long double x0, long double y0, long double x1, long double y1,
    long double c_re, long double c_im,
    int e,
    int width, int height,
    int maxIterations, int output[])
{
    static constexpr int MAX_THREADS = 32;

    if (numThreads > MAX_THREADS) {
        fprintf(stderr, "Error: Max allowed threads is %d\n", MAX_THREADS);
        exit(1);
    }

    std::thread workers[MAX_THREADS];
    JuliaWorkerArgs args[MAX_THREADS];

    int rowsPerThread = height / numThreads;
    int remainingRows = height % numThreads;

    int currentRow = 0;

    for (int i = 0; i < numThreads; i++) {
        int tmp_height = rowsPerThread + (i == numThreads - 1 ? remainingRows : 0);

        args[i].x0 = x0;
        args[i].x1 = x1;
        args[i].y0 = y0;
        args[i].y1 = y1;
        args[i].c_re = c_re;
        args[i].c_im = c_im;
        args[i].e = e;
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
        workers[i] = std::thread(juliaWorkerThreadStart, &args[i]);
    }

    juliaWorkerThreadStart(&args[0]);

    for (int i = 1; i < numThreads; i++) {
        workers[i].join();
    }
}