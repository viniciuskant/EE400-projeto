#include <cmath>
#include <complex>
#include <iostream>

// Julia set function with debug prints
static inline int julia(long double z0_re, long double z0_im, int count, long double c_re, long double c_im, int e = 2)
{
    long double z_re = z0_re;
    long double z_im = z0_im;
    int i;

    for (i = 0; i < count; ++i) {
        long double r2 = z_re * z_re + z_im * z_im;

        if (r2 > 4.0L) {
            break;
        }

        if (e == 2) {
            // Common case d=2: z -> z^2 + c
            long double new_re = z_re * z_re - z_im * z_im;
            long double new_im = 2.0L * z_re * z_im;
            z_re = new_re + c_re;
            z_im = new_im + c_im;
        } else {
            // General form: z -> z^d + c (with z in polar form)
            long double r = std::sqrt(r2);
            long double theta = std::atan2(z_im, z_re);
            long double r_d = std::pow(r, e);
            long double new_re = r_d * std::cos(e * theta);
            long double new_im = r_d * std::sin(e * theta);
            z_re = new_re + c_re;
            z_im = new_im + c_im;
        }

    }

    return i;
}


void juliaSerial(
    long double x0, long double y0, long double x1, long double y1,
    long double c_re, long double c_im,
    int e,
    int width, int height,
    int startRow, int totalRows,
    int maxIterations,
    int output[])
{
    long double dx = (x1 - x0) / width;
    long double dy = (y1 - y0) / height;

    int endRow = startRow + totalRows;

    for (int j = startRow; j < endRow; ++j) {
        for (int i = 0; i < width; ++i) {
            long double x = x0 + i * dx;
            long double y = y0 + j * dy;

            int index = j * width + i; // Adjust index for startRow offset
            output[index] = julia(x, y, maxIterations, c_re, c_im, e);
        }
    }
}