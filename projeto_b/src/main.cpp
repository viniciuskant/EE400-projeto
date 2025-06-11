#include <stdio.h>
#include <algorithm>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <png.h>
#include <unordered_set>

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


void writePNGImage(
    int* data,
    int width, int height,
    const char* filename,
    int maxIterations) {

    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open file %s for writing\n", filename);
        return;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fprintf(stderr, "Failed to create PNG write struct\n");
        fclose(fp);
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        fprintf(stderr, "Failed to create PNG info struct\n");
        png_destroy_write_struct(&png, NULL);
        fclose(fp);
        return;
    }

    if (setjmp(png_jmpbuf(png))) {
        fprintf(stderr, "Error during PNG creation\n");
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return;
    }

    png_init_io(png, fp);

    // Write header
    png_set_IHDR(
        png, info, width, height,
        8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png, info);

    // Write image data
    png_bytep row = (png_bytep)malloc(3 * width * sizeof(png_byte));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int value = data[y * width + x];
            float normalized = (float)value / maxIterations;

            // Map normalized value to a color gradient
            png_byte r, g, b;
            if (normalized < 0.25f) {
                r = (png_byte)(normalized * 4 * 255);
                g = 0;
                b = 0;
            } else if (normalized < 0.5f) {
                r = 255;
                g = (png_byte)((normalized - 0.25f) * 4 * 255);
                b = 0;
            } else if (normalized < 0.75f) {
                r = (png_byte)((1.0f - (normalized - 0.5f) * 4) * 255);
                g = 255;
                b = (png_byte)((normalized - 0.5f) * 4 * 255);
            } else {
                r = 0;
                g = (png_byte)((1.0f - (normalized - 0.75f) * 4) * 255);
                b = 255;
            }

            row[x * 3 + 0] = r;
            row[x * 3 + 1] = g;
            row[x * 3 + 2] = b;
        }
        png_write_row(png, row);
    }
    free(row);

    // End write
    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    fclose(fp);
}

int main(int argc, char** argv) {
    const unsigned int scale = 10;
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

    int* output_thread = new int[width * height];

    double minThread = 1e30;
    for (int i = 0; i < 5; ++i) {
        memset(output_thread, 0, width * height * sizeof(int));
        mandelbrotThread(numThreads, x0, y0, x1, y1, width, height, maxIterations, output_thread);
    }

    printf("[mandelbrot thread]:\t\t[%.3f] ms\n", minThread * 1000);
    writePNGImage(output_thread, width, height, "mandelbrot-thread.png", maxIterations);

    delete[] output_thread;

    return 0;
}
