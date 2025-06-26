#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "writePNG.h"


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
    const unsigned int scale = 100;
    const unsigned int width = 1600 * scale;
    const unsigned int height = 1200 * scale;
    const int maxIterations = 512;
    int numThreads = 2;

    float x0 = -2;
    float x1 = 1;
    float y0 = -1;
    float y1 = 1;

    int opt;
    int maxThreads = sysconf(_SC_NPROCESSORS_ONLN);
    numThreads = maxThreads > 0 ? maxThreads : 2;

    static struct option long_options[] = {
        {"threads", 1, 0, 't'},
        {"view", 1, 0, 'v'},
        {"help", 0, 0, '?'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "t:v:?", long_options, NULL)) != EOF) {
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
        mandelbrotThread(numThreads, x0, y0, x1, y1, width, height, maxIterations, output_thread);
    }

    printf("[mandelbrot thread]:\t\t[%.3f] ms\n", minThread * 1000);
    writePNGImage(output_thread, width, height, "../data/mandelbrot-thread.png", maxIterations);

    delete[] output_thread;

    return 0;
}
