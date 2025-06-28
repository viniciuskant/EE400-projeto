/*

  Note: This code was modified from example code
  originally provided by Intel.  To comply with Intel's open source
  licensing agreement, their copyright is retained below.

  -----------------------------------------------------------------

  Copyright (c) 2010-2011, Intel Corporation
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/




#include <cmath>

static inline int mandel(long double c_re, long double c_im, int count, long double z0_re = 0.0L, long double z0_im = 0.0L, int e = 7)
{
    long double z_re = z0_re;
    long double z_im = z0_im;
    int i;

    for (i = 0; i < count; ++i) {
        long double r2 = z_re * z_re + z_im * z_im;

        if (r2 > 4.0L)
            break;

        if (e == 2) {
            // Otimização para o caso comum d=2
            long double new_re = z_re * z_re - z_im * z_im;
            long double new_im = 2.0L * z_re * z_im;
            z_re = c_re + new_re;
            z_im = c_im + new_im;
        } else {
            // Forma polar: z = r * e^(iθ) => z^d = r^d * e^(i d θ)
            long double r = std::sqrt(r2);
            long double theta = std::atan2(z_im, z_re);
            long double r_d = std::pow(r, e);
            long double new_re = r_d * std::cos(e * theta);
            long double new_im = r_d * std::sin(e * theta);
            z_re = c_re + new_re;
            z_im = c_im + new_im;
        }
    }

    return i;
}

//
// MandelbrotSerial --
//
// Compute an image visualizing the mandelbrot set.  The resulting
// array contains the number of iterations required before the complex
// number corresponding to a pixel could be rejected from the set.
//
// * x0, y0, x1, y1 describe the complex coordinates mapping
//   into the image viewport.
// * width, height describe the size of the output image
// * startRow, totalRows describe how much of the image to compute
void mandelbrotSerial(
  long double x0, long double y0, long double x1, long double y1,
  long double z0_re, long double z0_im,
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

      int index = j * width + i;
      output[index] = mandel(x, y, maxIterations, z0_re, z0_im, e);
    }
  }
}

