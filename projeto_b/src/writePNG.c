#include "writePNG.h"

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