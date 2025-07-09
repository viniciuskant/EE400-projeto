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
#include <complex>
#include <vector>
#include <fstream>

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
    long double radius,
    int width, int height,
    int maxIterations,
    int output[]);

extern void juliaThread(
    int numThreads,
    long double x0, long double y0, long double x1, long double y1,
    long double c_re, long double c_im,
    int e,
    int width, int height,
    int maxIterations, int output[]);

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
    printf("  -t, --threads <N>         Specify the number of threads to use (default: system's max threads)\n");
    printf("  -v, --view <INT>          Select a predefined view (1: default, 2: zoomed-in view)\n");
    printf("  -m, --max-iterations <N>  Set the maximum number of iterations for the fractal computation (default: 750)\n");
    printf("  -z, --z0 <real,imaginary> Specify the initial complex value z0 (default: 0,0)\n");
    printf("  -e, --exponent <N>        Set the exponent value for the fractal computation (default: 2)\n");
    printf("  -j, --julia <real,imaginary> Enable Julia set mode with specified complex value (default: disabled)\n");
    printf("  -r, --radius <FLOAT>      Set the radius for the fractal computation (default: 2.0)\n");
    printf("  -q, --questoes            Execute predefined questions (e.g., questao_1a, questao_2_parte1)\n");
    printf("  -?, --help                Display this help message and exit\n");
    printf("\nExamples:\n");
    printf("  %s -t 4 -v 2 -m 1000 -z 0.355,0.355 -e 3\n", progname);
    printf("  %s --threads 8 --view 1 --max-iterations 500\n", progname);
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


void questao_2_parte1(int width, int height, long double x_min, long double x_max, long double y_min, long double y_max) {
    std::vector<int> buffer(width * height, 0);

    #pragma omp parallel for collapse(2)
    for (int py = 0; py < height; ++py) {
        for (int px = 0; px < width; ++px) {
            long double x = x_min + (x_max - x_min) * px / (width - 1);
            long double y = y_min + (y_max - y_min) * py / (height - 1);

            std::complex<long double> c(x, y);
            std::complex<long double> sqrt_disc = std::sqrt(1.0L - 4.0L * c);
            std::complex<long double> root1 = (1.0L + sqrt_disc) / 2.0L;
            std::complex<long double> root2 = (1.0L - sqrt_disc) / 2.0L;

            if (std::abs(root1) < 0.5L || std::abs(root2) < 0.5L) {
                buffer[py * width + px] = 128;
            }
        }
    }

    writePNGImage(buffer.data(), width, height, "data/questao_2_parte1.png", 1);
    printf("Region plot saved to data/questao_2_parte1.png\n");
}

void questao_2_parte2(int width, int height, long double x_min, long double x_max, long double y_min, long double y_max) {
    std::vector<int> buffer(width * height, 0);
    float tolerancia = 0.01;
    long double step = 0.2L;

    #pragma omp parallel for collapse(2)
    for (int py = 0; py < height; ++py) {
        for (int px = 0; px < width; ++px) {
            long double x = x_min + (x_max - x_min) * px / (width - 1);
            long double y = y_min + (y_max - y_min) * py / (height - 1);

            std::complex<long double> c(x, y);

            for (long double z_re = -2.0L; z_re < 2.0L; z_re += step) {
                for (long double z_im = -2.0L; z_im < 2.0L; z_im += step) {
                    std::complex<long double> z(z_re, z_im);
                    std::complex<long double> equation = 4.0L * z * (z * z + c);

                    if (std::abs(equation) < 1.0L) { // satisfez a primeira parte
                        equation = (z * z + c) * (z * z + c) + c - z;
                        if (std::abs(equation) < tolerancia) {
                            buffer[py * width + px] = 128;
                            break; // Seta e sai do loop interno
                        }\
                    }
                }
            }
        }
    }

    writePNGImage(buffer.data(), width, height, "data/questao_2_parte2.png", 1);
    printf("Region plot saved to data/questao_2_parte2.png\n");
}

int main(int argc, char** argv) {
    // Default configuration values
    struct Config {
        long double x0 = -2.0;
        long double x1 = 1.5;
        long double y0 = -1.5;
        long double y1 = 1.5;
        long double j_re = 0.0;
        long double j_im = 0.0;
        long double z0_re = 0.0;
        long double z0_im = 0.0;
        long double radius = 2.0;
        bool juliaMode = false;
        unsigned int scale = 1;
        int maxIterations = 750;
        int exponent = 2;
        int numThreads = sysconf(_SC_NPROCESSORS_ONLN);
    } config;

    // Calculate dimensions based on configuration
    const unsigned int width = static_cast<unsigned int>((config.x1 - config.x0) * 1000 * config.scale);
    const unsigned int height = static_cast<unsigned int>((config.y1 - config.y0) * 1000 * config.scale);

    // Validate thread count
    if (config.numThreads <= 0) {
        config.numThreads = 4;
    }

    // Command line options
    static struct option long_options[] = {
        {"threads", required_argument, 0, 't'},
        {"view", required_argument, 0, 'v'},
        {"z0", required_argument, 0, 'z'},
        {"maxIterations", required_argument, 0, 'm'},
        {"exponent", required_argument, 0, 'e'},
        {"julia", required_argument, 0, 'j'},
        {"radius", required_argument, 0, 'r'},
        {"questoes", no_argument, 0, 'q'},
        {"help", no_argument, 0, '?'},
        {0, 0, 0, 0}
    };

    // Parse command line arguments
    int opt;
    while ((opt = getopt_long(argc, argv, "t:v:z:m:e:j?", long_options, NULL)) != -1) {
        switch (opt) {
            case 't':
                config.numThreads = atoi(optarg);
                if (config.numThreads <= 0) {
                    fprintf(stderr, "Number of threads must be greater than 0\n");
                    return 1;
                }
                break;
                
            case 'v': {
                int viewIndex = atoi(optarg);
                if (viewIndex == 2) {
                    scaleAndShift(config.x0, config.x1, config.y0, config.y1, 0.015f, -0.986f, 0.30f);
                } else if (viewIndex > 1) {
                    fprintf(stderr, "Invalid view index\n");
                    return 1;
                }
                break;
            }
                
            case 'm':
                config.maxIterations = atoi(optarg);
                if (config.maxIterations <= 0) {
                    fprintf(stderr, "Max iterations must be greater than 0\n");
                    return 1;
                }
                break;
                
            case 'e':
                config.exponent = atoi(optarg);
                printf("Exponent set to: %d\n", config.exponent);
                break;
                
            case 'j': {
                config.juliaMode = true;
                const char* julia_params = optarg;
                
                // Debug adicional
                for (int i = 0; i < argc; i++) {
                    printf("argv[%d] = %s\n", i, argv[i]);
                }
                
                // Se optarg é NULL, tenta pegar o próximo argumento
                if (julia_params == NULL && optind < argc) {
                    julia_params = argv[optind];
                    // Verifica se não é outra opção
                    if (julia_params[0] == '-') {
                        julia_params = NULL;
                    } else {
                        optind++;
                    }
                }
                
                if (julia_params == NULL) {
                    fprintf(stderr, "Error: Missing Julia parameters after -j\n");
                    return 1;
                }
                
                
                if (sscanf(julia_params, "%Lf,%Lf", &config.j_re, &config.j_im) != 2) {
                    fprintf(stderr, "Error: Invalid Julia parameters format. Expected 'real,imaginary'\n");
                    fprintf(stderr, "Example: -j 0.285,0.01\n");
                    return 1;
                }
                
                printf("Julia set enabled with c = (%Lf, %Lf)\n", config.j_re, config.j_im);
                break;
            }
            case 'r':
                config.radius = atof(optarg);
                if (config.radius <= 0) {
                    fprintf(stderr, "Radius must be greater than 0\n");
                    return 1;
                }
                break;
            case 'q': 
                questao_1a();
                questao_2_parte1(width, height, config.x0, config.x1, config.y0, config.y1);
                questao_2_parte2(width, height, config.x0, config.x1, config.y0, config.y1);
                break;
            case 'z':
                if (sscanf(optarg, "%Lf,%Lf", &config.z0_re, &config.z0_im) != 2) {
                    fprintf(stderr, "Invalid format for z0. Use --z0 <real,imaginary>\n");
                    return 1;
                }
                break;
                
            case '?':
            default:
                usage(argv[0]);
                return 1;
        }
    }

    // Print configuration
    printf("Using %d threads\n", config.numThreads);
    printf("Configuration:\n");
    printf("  Threads: %d\n", config.numThreads);
    printf("  Resolution: %ux%u\n", width, height);
    printf("  Max Iterations: %d\n", config.maxIterations);
    printf("  Viewport: x0 = %Lf, x1 = %Lf, y0 = %Lf, y1 = %Lf\n", 
           config.x0, config.x1, config.y0, config.y1);
    printf("  Initial z0: (%Lf, %Lf)\n", config.z0_re, config.z0_im);
    printf("  Exponent: %d\n", config.exponent);

    questao_2_parte1(width, height, config.x0, config.x1, config.y0, config.y1);

    // Start timing
    auto mainStart = std::chrono::high_resolution_clock::now();
    auto computationStart = std::chrono::high_resolution_clock::now();

    // Allocate output buffer
    int* output = new int[width * height];

    // Compute fractal
    if (!config.juliaMode) {
        // Optimize for symmetric case
        if (config.z0_im == 0) {
            mandelbrotThread(config.numThreads, config.x0, config.y0, config.x1, 0, 
                             config.z0_re, config.z0_im, config.exponent, config.radius, width, 
                             height / 2, config.maxIterations, output);
            
            // Mirror the upper half to the lower half
            #pragma omp parallel for num_threads(config.numThreads) collapse(2)
            for (unsigned int y = 0; y < height / 2; ++y) {
                for (unsigned int x = 0; x < width; ++x) {
                    output[(height - 1 - y) * width + x] = output[y * width + x];
                }
            }
        } else {
            mandelbrotThread(config.numThreads, config.x0, config.y0, config.x1, config.y1,
                             config.z0_re, config.z0_im, config.exponent, config.radius, width, 
                             height, config.maxIterations, output);
        }
    } else {
        juliaThread(config.numThreads, config.x0, config.y0, config.x1, config.y1,
                config.j_re, config.j_im, config.exponent, width,
                height, config.maxIterations, output);
    }

    // End timing and print results
    auto computationEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> computationTime = computationEnd - computationStart;
    
    if (computationTime.count() < 1) {
        printf("[Computation time]:\t[%.3f] ms\n", computationTime.count() * 1000);
    } else if (computationTime.count() < 60) {
        printf("[Computation time]:\t[%.3f] s\n", computationTime.count());
    } else {
        int minutes = static_cast<int>(computationTime.count() / 60);
        double seconds = computationTime.count() - minutes * 60;
        printf("[Computation time]:\t[%d min %.3f s]\n", minutes, seconds);
    }

    // Determine output directory
    const char* baseDir = "data";
    if (access(baseDir, F_OK) != 0 && mkdir(baseDir, 0755) != 0) {
        fprintf(stderr, "Error: Could not create base directory %s\n", baseDir);
        delete[] output;
        return 1;
    }

    // Create appropriate subdirectories
    const char* fractalType = config.juliaMode ? "julia" : "mandelbrot";
    char fractalDir[256], exponentDir[1024];
    
    snprintf(fractalDir, sizeof(fractalDir), "%s/%s", baseDir, fractalType);
    if (access(fractalDir, F_OK) != 0 && mkdir(fractalDir, 0755) != 0) {
        fprintf(stderr, "Error creating directory %s\n", fractalDir);
        delete[] output;
        return 1;
    }

    snprintf(exponentDir, sizeof(exponentDir), "%s/exponent-%d", fractalDir, config.exponent);
    if (access(exponentDir, F_OK) != 0 && mkdir(exponentDir, 0755) != 0) {
        fprintf(stderr, "Error creating directory %s\n", exponentDir);
        delete[] output;
        return 1;
    }

    // Save image
    char filename[2048];
    if (config.juliaMode) {
        snprintf(filename, sizeof(filename), "%s/julia-%d-iterations-c(%Lf,%Lf)-radius(%Lf).png", 
                 exponentDir, config.maxIterations, config.j_re, config.j_im, config.radius);
    } else {
        snprintf(filename, sizeof(filename), "%s/mandelbrot-%d-iterations-z0(%Lf,%Lf)-radius(%Lf).png", 
                 exponentDir, config.maxIterations, config.z0_re, config.z0_im, config.radius);
    }
    
    writePNGImage(output, width, height, filename, config.maxIterations);
    printf("Image saved to: %s\n", filename);

    // Clean up
    delete[] output;

    // Print total execution time
    auto mainEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> totalTime = mainEnd - mainStart;
    printf("Total execution time: %.3f seconds\n", totalTime.count());

    return 0;
}