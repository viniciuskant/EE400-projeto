#include <math.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <chrono>
#include <omp.h>
#include "lodepng.h"

void writePNGImage(int* buffer, unsigned int width, unsigned int height, const char* filename, int maxIterations) {
    std::vector<unsigned char> image(width * height * 4);

    #pragma omp parallel for collapse(2)
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            int value = buffer[y * width + x];
            float normalized = static_cast<float>(value) / maxIterations;

            // Map normalized value to a high-detail color gradient
            unsigned char r, g, b;
            if (normalized < 0.16f) {
                r = 0;
                g = 0;
                b = static_cast<unsigned char>(normalized * 6.25f * 255);
            } else if (normalized < 0.33f) {
                r = 255;
                g = static_cast<unsigned char>((normalized - 0.16f) * 6.25f * 255);
                b = 0;
            } else if (normalized < 0.5f) {
                r = static_cast<unsigned char>((1.0f - (normalized - 0.33f) * 6.25f) * 255);
                g = 255;
                b = static_cast<unsigned char>((normalized - 0.33f) * 6.25f * 255);
            } else if (normalized < 0.66f) {
                r = 0;
                g = static_cast<unsigned char>((1.0f - (normalized - 0.5f) * 6.25f) * 255);
                b = 255;
            } else if (normalized < 0.83f) {
                r = static_cast<unsigned char>((normalized - 0.66f) * 6.25f * 255);
                g = 0;
                b = 255;
            } else {
                r = 255;
                g = static_cast<unsigned char>((normalized - 0.83f) * 6.25f * 255);
                b = static_cast<unsigned char>((1.0f - (normalized - 0.83f) * 6.25f) * 255);
            }

            unsigned int index = 4 * (y * width + x);
            image[index + 0] = r; // Red
            image[index + 1] = g; // Green
            image[index + 2] = b; // Blue
            image[index + 3] = 255; // Alpha
        }
    }

    unsigned error = lodepng::encode(filename, image, width, height);
    if (error) {
        fprintf(stderr, "Error encoding PNG: %s\n", lodepng_error_text(error));
    }
}

extern void mandelbrotThread(
    int numThreads,
    long double x0, long double y0, long double x1, long double y1,
    long double z0_re, long double z0_im,
    int e,
    int width, int height,
    int maxIterations,
    int output[]);

void scaleAndShift(long double& x0, long double& x1, long double& y0, long double& y1,
                   long double scale,
                   long double shiftX, long double shiftY) {
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
    printf("  -t, --threads <N>         Use N threads\n");
    printf("  -v, --view <INT>          Use specified view settings\n");
    printf("  -m, --max-iterations <N>  Set the maximum number of iterations\n");
    printf("  -z, --z0 <real,imaginary> Set the initial value of z0\n");
    printf("  -e, --exponent <N>        Set the exponent value\n");
    printf("  -?, --help                Display this help message\n");
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
    long double x0 = -2;
    long double x1 = 1.5;
    long double y0 = -1.5;
    long double y1 = 1.5;
    const unsigned int scale = 2;
    const unsigned int width = static_cast<unsigned int>((x1 - x0) * 1000 * scale);
    const unsigned int height = static_cast<unsigned int>((y1 - y0) * 1000 * scale);
    int maxIterations = 750;
    int exponent = 2;

    int numThreads = sysconf(_SC_NPROCESSORS_ONLN);
    if (numThreads <= 0) {
        numThreads = 4;
    }


    long double z0_re =0 ,  z0_im = 0;


    int opt;
    int maxThreads = sysconf(_SC_NPROCESSORS_ONLN);
    numThreads = maxThreads > 0 ? maxThreads : 2;

    static struct option long_options[] = {
        {"threads", required_argument, 0, 't'},
        {"view", required_argument, 0, 'v'},
        {"z0", required_argument, 0, 'z'},
        {"maxIterations", required_argument, 0, 'm'},
        {"exponent", required_argument, 0, 'e'},
        {"help", no_argument, 0, '?'},
        {0, 0, 0, 0}
    };

    printf("Using %d threads\n", numThreads);

    while ((opt = getopt_long(argc, argv, "t:v:z:?", long_options, NULL)) != -1) {
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
        case 'm': {
            maxIterations = atoi(optarg);
            if (maxIterations <= 0) {
            fprintf(stderr, "Max iterations must be greater than 0\n");
            return 1;
            }
            break;
        }
        case 'e': {
            exponent = atoi(optarg);
            // if (exponent <= 0) {
            //     fprintf(stderr, "Exponent must be greater than 0\n");
            //     return 1;
            // }
            printf("Exponent set to: %d\n", exponent);
            break;
        }
        case 'z': {
            if (sscanf(optarg, "%Lf,%Lf", &z0_re, &z0_im) != 2) {
                fprintf(stderr, "Invalid format for z0. Use --z0 <real,imaginary>\n");
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

    auto mainStart = std::chrono::high_resolution_clock::now();

    // questao_1a();

    int* output_thread = new int[width * height];

    auto start = std::chrono::high_resolution_clock::now();


    printf("Configuration:\n");
    printf("  Threads: %d\n", numThreads);
    printf("  Resolution: %ux%u\n", width, height);
    printf("  Max Iterations: %d\n", maxIterations);
    printf("  Viewport: x0 = %Lf, x1 = %Lf, y0 = %Lf, y1 = %Lf\n", x0, x1, y0, y1);
    printf("  Initial z0: (%Lf, %Lf)\n", z0_re, z0_im);
    printf("  Exponent: %d\n", exponent);

    // Devido ao item b do exercício 1, que provo a simetria do conjunto de Mandelbrot, faço essa otimização
    // Ajusta mandelbrotThread para calcular apenas metade dos pontos e usar simetria
    
    if (z0_im == 0){

        mandelbrotThread(numThreads, x0, y0, x1, 0, z0_re, z0_im, exponent, width, height / 2, maxIterations, output_thread);
        
        // Copia a metade superior para a metade inferior usando simetria
        #pragma omp parallel for num_threads(numThreads) collapse(2)
        for (unsigned int y = 0; y < height / 2; ++y) {
            for (unsigned int x = 0; x < width; ++x) {
                output_thread[(height - 1 - y) * width + x] = output_thread[y * width + x];
            }
        }
    } else{
        mandelbrotThread(numThreads, x0, y0, x1, y1, z0_re, z0_im, exponent, width, height, maxIterations, output_thread);

    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> totalElapsed = end - start;
    double totalTimeInSeconds = totalElapsed.count();

    if (totalTimeInSeconds < 1) {
        printf("[mandelbrot thread]:\t[%.3f] ms\n", totalTimeInSeconds * 1000);
    } else if (totalTimeInSeconds < 60) {
        printf("[mandelbrot thread]:\t[%.3f] s\n", totalTimeInSeconds);
    } else {
        int minutes = static_cast<int>(totalTimeInSeconds / 60);
        double seconds = totalTimeInSeconds - minutes * 60;
        printf("[mandelbrot thread total]:\t[%d min %.3f s]\n", minutes, seconds);
    }

    // Determine the directory to save the image
    const char* directory = nullptr;
    if (access("data", F_OK) == 0) {
        directory = "data";
    } else if (access("../data", F_OK) == 0) {
        directory = "../data";
    } else {
        fprintf(stderr, "Error: Could not find a valid directory to save the image.\n");
        delete[] output_thread;
        return 1;
    }

    // Create a directory for the current exponent if it doesn't exist
    char exponentDir[256];
    snprintf(exponentDir, sizeof(exponentDir), "%s/exponent-%d", directory, exponent);
    if (access(exponentDir, F_OK) != 0) {
        if (mkdir(exponentDir, 0755) != 0) {
            fprintf(stderr, "Error: Could not create directory %s\n", exponentDir);
            delete[] output_thread;
            return 1;
        }
    }

    // Construct the filename with iteration count and z0 information
    char filename[512];
    snprintf(filename, sizeof(filename), "%s/mandelbrot-%d-iterations-z0(%Lf,%Lf).png", 
             exponentDir, maxIterations, z0_re, z0_im);

    // Save the image
    writePNGImage(output_thread, width, height, filename, maxIterations);
    printf("Image saved to: %s\n", filename);

    delete[] output_thread;

    auto mainEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> mainElapsed = mainEnd - mainStart;
    printf("Total execution time of main: %.3f seconds\n", mainElapsed.count());

    return 0;
}
