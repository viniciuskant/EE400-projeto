#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "writePNG.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <unistd.h>
#include <chrono>
#include <omp.h>


extern void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations,
    int output[]);

void scaleAndShift(float& x0, float& x1, float& y0, float& y1,
                   float scale,
                   float shiftX, float shiftY) {
    x0 *= scale;
    x1 *= scale;
    y0 *= scale;
    y1 *= scale;
    x0 += shiftX;
    x1 += shiftX;
    y0 += shiftY;
    y1 += shiftY;
}

void usage(const char* progname) {
    printf("Usage: %s [options]\n", progname);
    printf("Program Options:\n");
    printf("  -t  --threads <N>  Use N threads\n");
    printf("  -v  --view <INT>   Use specified view settings\n");
    printf("  -?  --help         This message\n");
}

void questao_1a() {
    auto printOrbit = [](float c_re, float c_im, int count) {
        printf("Orbit for c = (%f, %f):\n", c_re, c_im);
        float z_re = c_re, z_im = c_im;
        for (int i = 0; i < count; ++i) {
            printf("z%d = (%f, %f)\n", i, z_re, z_im);
            if (z_re * z_re + z_im * z_im > 4.f) {
                printf("Escaped at iteration %d\n", i);
                break;
            }
            float new_re = z_re * z_re - z_im * z_im;
            float new_im = 2.f * z_re * z_im;
            z_re = c_re + new_re;
            z_im = c_im + new_im;
        }
    };

    // Test points
    printOrbit(-2.0f, 0.0f, 10); // c1 = -2
    printOrbit(0.0f, -2.0f, 10); // c2 = -2i
    float c3_re = 0.35f * cos(M_PI / 4);
    float c3_im = 0.35f * sin(M_PI / 4);
    printOrbit(c3_re, c3_im, 10); // c3 = 0.35 * e^(iπ/4)
}

int main(int argc, char** argv) { 
    const unsigned int scale = 2;
    const unsigned int width = 7680 * scale;
    const unsigned int height = 4320 * scale;
    const int maxIterations = 500;
    int numThreads = 8;

    float x0 = -2;
    float x1 = 1;
    float y0 = -1;
    float y1 = 1;

    int opt;
    int maxThreads = sysconf(_SC_NPROCESSORS_ONLN);
    numThreads = maxThreads > 0 ? maxThreads : 2;

    static struct option long_options[] = {
        {"threads", required_argument, 0, 't'},
        {"view", required_argument, 0, 'v'},
        {"help", no_argument, 0, '?'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "t:v:?", long_options, NULL)) != -1) {
        switch (opt) {
        case 't': {
            numThreads = atoi(optarg);
            if (numThreads <= 0) {
                fprintf(stderr, "Number of threads must be greater than 0\n");
                return 1;
            }
            break;
        }
        case 'v': {
            int viewIndex = atoi(optarg);
            if (viewIndex == 2) {
                float scaleValue = .015f;
                float shiftX = -.986f;
                float shiftY = .30f;
                scaleAndShift(x0, x1, y0, y1, scaleValue, shiftX, shiftY);
            } else if (viewIndex > 1) {
                fprintf(stderr, "Invalid view index\n");
                return 1;
            }
            break;
        }
        case '?':
        default:
            usage(argv[0]);
            return 1;
        }
    }

    questao_1a();

    int* output_thread = new int[width * height];

    double minThread = 1e30;
    for (int i = 0; i < 5; ++i) {
        memset(output_thread, 0, width * height * sizeof(int));

        auto start = std::chrono::high_resolution_clock::now();

        // Devido ao item b do exercício 1, que provo a simetria do conjunto de Mandelbrot, faço essa otimização
        // Ajusta mandelbrotThread para calcular apenas metade dos pontos e usar simetria
        mandelbrotThread(numThreads, x0, y0, x1, 0, width, height / 2, maxIterations, output_thread);

        // Copia a metade superior para a metade inferior usando simetria
        #pragma omp parallel for num_threads(numThreads) collapse(2)
        for (unsigned int y = 0; y < height / 2; ++y) {
            for (unsigned int x = 0; x < width; ++x) {
                output_thread[(height - 1 - y) * width + x] = output_thread[y * width + x];
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        double timeInSeconds = elapsed.count();

        if (timeInSeconds < minThread) {
            minThread = timeInSeconds;
        }
    }

    if (minThread < 1) {
        printf("[mandelbrot thread]:\t\t[%.3f] ms\n", minThread * 1000);
    } else if (minThread < 60) {
        printf("[mandelbrot thread]:\t\t[%.3f] s\n", minThread);
    } else {
        int minutes = static_cast<int>(minThread / 60);
        double seconds = minThread - minutes * 60;
        printf("[mandelbrot thread]:\t\t[%d min %.3f s]\n", minutes, seconds);
    }

    writePNGImage(output_thread, width, height, "../data/mandelbrot-thread.png", maxIterations);

    delete[] output_thread;

    return 0;
}
