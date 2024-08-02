/*

  ESP32 FFT
  =========

  This provides a vanilla radix-2 FFT implementation and a test example.

  Author
  ------

  This code was written by [Robin Scheibler](http://www.robinscheibler.org) during rainy days in October 2017.

  License
  -------

  Copyright (c) 2017 Robin Scheibler

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>

#include "fft.h"
#include "py/runtime.h"
#include "esp_err.h"
#include "esp_log.h"
#define TWO_PI 6.28318530
#define USE_SPLIT_RADIX 1
#define LARGE_BASE_CASE 1
static const char *TAG = "lvgl_esp32_fft";

fft_config_t *fft_init(int size, fft_type_t type, fft_direction_t direction, float *input, float *output)
{
  /*
   * Prepare an FFT of correct size and types.
   *
   * If no input or output buffers are provided, they will be allocated.
   */
  int k,m;

  fft_config_t *config = (fft_config_t *)malloc(sizeof(fft_config_t));

  // Check if the size is a power of two
  if ((size & (size-1)) != 0)  // tests if size is a power of two
    return NULL;

  // start configuration
  config->flags = 0;
  config->type = type;
  config->direction = direction;
  config->size = size;
  config->input = NULL;
  config->output = NULL;
  // Allocate and precompute twiddle factors
  config->twiddle_factors = (float *)malloc(2 * config->size * sizeof(float));

  float two_pi_by_n = TWO_PI / config->size;

  for (k = 0, m = 0 ; k < config->size ; k++, m+=2)
  {
    config->twiddle_factors[m] = cosf(two_pi_by_n * k);    // real
    config->twiddle_factors[m+1] = sinf(two_pi_by_n * k);  // imag
  }

  // Allocate input buffer
  if (input != NULL)
    config->input = input;
  else 
  {
    if (config->type == FFT_REAL)
      config->input = (float *)malloc(config->size * sizeof(float));
    else if (config->type == FFT_COMPLEX)
      config->input = (float *)malloc(2 * config->size * sizeof(float));

    config->flags |= FFT_OWN_INPUT_MEM;
  }

  if (config->input == NULL)
    return NULL;

  // Allocate output buffer
  if (output != NULL)
    config->output = output;
  else
  {
    if (config->type == FFT_REAL)
      config->output = (float *)malloc(config->size * sizeof(float));
    else if (config->type == FFT_COMPLEX)
      config->output = (float *)malloc(2 * config->size * sizeof(float));

    config->flags |= FFT_OWN_OUTPUT_MEM;
  }

  if (config->output == NULL)
    return NULL;

  return config;
}

void fft_destroy(fft_config_t *config)
{
  if (config->flags & FFT_OWN_INPUT_MEM)
    free(config->input);

  if (config->flags & FFT_OWN_OUTPUT_MEM)
    free(config->output);

  free(config->twiddle_factors);
  free(config);
}

void fft_execute(fft_config_t *config)
{
  if (config->type == FFT_REAL && config->direction == FFT_FORWARD)
    rfft(config->input, config->output, config->twiddle_factors, config->size);
  else if (config->type == FFT_REAL && config->direction == FFT_BACKWARD)
    irfft(config->input, config->output, config->twiddle_factors, config->size);
  else if (config->type == FFT_COMPLEX && config->direction == FFT_FORWARD)
    fft(config->input, config->output, config->twiddle_factors, config->size);
  else if (config->type == FFT_COMPLEX && config->direction == FFT_BACKWARD)
    ifft(config->input, config->output, config->twiddle_factors, config->size);
}

void fft(float *input, float *output, float *twiddle_factors, int n)
{
  /*
   * Forward fast Fourier transform
   * DIT, radix-2, out-of-place implementation
   *
   * Parameters
   * ----------
   *  input (float *)
   *    The input array containing the complex samples with
   *    real/imaginary parts interleaved [Re(x0), Im(x0), ..., Re(x_n-1), Im(x_n-1)]
   *  output (float *)
   *    The output array containing the complex samples with
   *    real/imaginary parts interleaved [Re(x0), Im(x0), ..., Re(x_n-1), Im(x_n-1)]
   *  n (int)
   *    The FFT size, should be a power of 2
   */

#if USE_SPLIT_RADIX
  split_radix_fft(input, output, n, 2, twiddle_factors, 2);
#else
  fft_primitive(input, output, n, 2, twiddle_factors, 2);
#endif
}

void ifft(float *input, float *output, float *twiddle_factors, int n)
{
  /*
   * Inverse fast Fourier transform
   * DIT, radix-2, out-of-place implementation
   *
   * Parameters
   * ----------
   *  input (float *)
   *    The input array containing the complex samples with
   *    real/imaginary parts interleaved [Re(x0), Im(x0), ..., Re(x_n-1), Im(x_n-1)]
   *  output (float *)
   *    The output array containing the complex samples with
   *    real/imaginary parts interleaved [Re(x0), Im(x0), ..., Re(x_n-1), Im(x_n-1)]
   *  n (int)
   *    The FFT size, should be a power of 2
   */
  ifft_primitive(input, output, n, 2, twiddle_factors, 2);
}

void rfft(float *x, float *y, float *twiddle_factors, int n)
{

  // This code uses the two-for-the-price-of-one strategy
#if USE_SPLIT_RADIX
  split_radix_fft(x, y, n / 2, 2, twiddle_factors, 4);
#else
  fft_primitive(x, y, n / 2, 2, twiddle_factors, 4);
#endif

  // Now apply post processing to recover positive
  // frequencies of the real FFT
  float t = y[0];
  y[0] = t + y[1];  // DC coefficient
  y[1] = t - y[1];  // Center coefficient

  // Apply post processing to quarter element
  // this boils down to taking complex conjugate
  y[n/2+1] = -y[n/2+1];

  // Now process all the other frequencies
  int k;
  for (k = 2 ; k < n / 2 ; k += 2)
  {
    float xer, xei, xor, xoi, c, s, tr, ti;

    c = twiddle_factors[k];
    s = twiddle_factors[k+1];
    
    // even half coefficient
    xer = 0.5 * (y[k] + y[n-k]);
    xei = 0.5 * (y[k+1] - y[n-k+1]);

    // odd half coefficient
    xor = 0.5 * (y[k+1] + y[n-k+1]);
    xoi = - 0.5 * (y[k] - y[n-k]);

    tr =  c * xor + s * xoi;
    ti = -s * xor + c * xoi;

    y[k]   = xer + tr;
    y[k+1] = xei + ti;

    y[n-k]   =   xer - tr;
    y[n-k+1] = -(xei - ti);
  }
}

void irfft(float *x, float *y, float *twiddle_factors, int n)
{
  /*
   * Destroys content of input vector
   */
  int k;

  // Here we need to apply a pre-processing first
  float t = x[0];
  x[0] = 0.5 * (t + x[1]);
  x[1] = 0.5 * (t - x[1]);

  x[n/2+1] = -x[n/2+1];

  for (k = 2 ; k < n / 2 ; k += 2)
  {
    float xer, xei, xor, xoi, c, s, tr, ti;

    c = twiddle_factors[k];
    s = twiddle_factors[k+1];

    xer = 0.5 * (x[k] + x[n-k]);
    tr  = 0.5 * (x[k] - x[n-k]);

    xei = 0.5 * (x[k+1] - x[n-k+1]);
    ti  = 0.5 * (x[k+1] + x[n-k+1]);

    xor = c * tr - s * ti;
    xoi = s * tr + c * ti;

    x[k]   = xer - xoi;
    x[k+1] = xor + xei;

    x[n-k]   = xer + xoi;
    x[n-k+1] = xor - xei;
  }

  ifft_primitive(x, y, n / 2, 2, twiddle_factors, 4);
}

void fft_primitive(float *x, float *y, int n, int stride, float *twiddle_factors, int tw_stride)
{
  /*
   * This code will compute the FFT of the input vector x
   *
   * The input data is assumed to be real/imag interleaved
   *
   * The size n should be a power of two
   *
   * y is an output buffer of size 2n to accomodate for complex numbers
   *
   * Forward fast Fourier transform
   * DIT, radix-2, out-of-place implementation
   *
   * For a complex FFT, call first stage as:
   * fft(x, y, n, 2, 2);
   *
   * Parameters
   * ----------
   *  x (float *)
   *    The input array containing the complex samples with
   *    real/imaginary parts interleaved [Re(x0), Im(x0), ..., Re(x_n-1), Im(x_n-1)]
   *  y (float *)
   *    The output array containing the complex samples with
   *    real/imaginary parts interleaved [Re(x0), Im(x0), ..., Re(x_n-1), Im(x_n-1)]
   *  n (int)
   *    The FFT size, should be a power of 2
   *  stride (int)
   *    The number of elements to skip between two successive samples
   *  tw_stride (int)
   *    The number of elements to skip between two successive twiddle factors
   */
  int k;
  float t;

#if LARGE_BASE_CASE
  // End condition, stop at n=8 to avoid one trivial recursion
  if (n == 8)
  {
    fft8(x, stride, y, 2);
    return;
  }
#else
  // End condition, stop at n=2 to avoid one trivial recursion
  if (n == 2)
  {
    y[0] = x[0] + x[stride];
    y[1] = x[1] + x[stride + 1];
    y[2] = x[0] - x[stride];
    y[3] = x[1] - x[stride + 1];
    return;
  }
#endif

  // Recursion -- Decimation In Time algorithm
  fft_primitive(x, y, n / 2, 2 * stride, twiddle_factors, 2 * tw_stride);             // even half
  fft_primitive(x + stride, y+n, n / 2, 2 * stride, twiddle_factors, 2 * tw_stride);  // odd half

  // Stitch back together

  // We can a few multiplications in the first step
  t = y[0];
  y[0] = t + y[n];
  y[n] = t - y[n];

  t = y[1];
  y[1] = t + y[n+1];
  y[n+1] = t - y[n+1];

  for (k = 1 ; k < n / 2 ; k++)
  {
    float x1r, x1i, x2r, x2i, c, s;
    c = twiddle_factors[k * tw_stride];
    s = twiddle_factors[k * tw_stride + 1];

    x1r = y[2 * k];
    x1i = y[2 * k + 1];
    x2r =  c * y[n + 2 * k] + s * y[n + 2 * k + 1];
    x2i = -s * y[n + 2 * k] + c * y[n + 2 * k + 1];

    y[2 * k] = x1r + x2r;
    y[2 * k + 1] = x1i + x2i;

    y[n + 2 * k] = x1r - x2r;
    y[n + 2 * k + 1] = x1i - x2i;
  }

}

void split_radix_fft(float *x, float *y, int n, int stride, float *twiddle_factors, int tw_stride)
{
  /*
   * This code will compute the FFT of the input vector x
   *
   * The input data is assumed to be real/imag interleaved
   *
   * The size n should be a power of two
   *
   * y is an output buffer of size 2n to accomodate for complex numbers
   *
   * Forward fast Fourier transform
   * Split-Radix
   * DIT, radix-2, out-of-place implementation
   *
   * For a complex FFT, call first stage as:
   * fft(x, y, n, 2, 2);
   *
   * Parameters
   * ----------
   *  x (float *)
   *    The input array containing the complex samples with
   *    real/imaginary parts interleaved [Re(x0), Im(x0), ..., Re(x_n-1), Im(x_n-1)]
   *  y (float *)
   *    The output array containing the complex samples with
   *    real/imaginary parts interleaved [Re(x0), Im(x0), ..., Re(x_n-1), Im(x_n-1)]
   *  n (int)
   *    The FFT size, should be a power of 2
   *  stride (int)
   *    The number of elements to skip between two successive samples
   *  twiddle_factors (float *)
   *    The array of twiddle factors
   *  tw_stride (int)
   *    The number of elements to skip between two successive twiddle factors
   */
  int k;

#if LARGE_BASE_CASE
  // End condition, stop at n=2 to avoid one trivial recursion
  if (n == 8)
  {
    fft8(x, stride, y, 2);
    return;
  }
  else if (n == 4)
  {
    fft4(x, stride, y, 2);
    return;
  }
#else
  // End condition, stop at n=2 to avoid one trivial recursion
  if (n == 2)
  {
    y[0] = x[0] + x[stride];
    y[1] = x[1] + x[stride + 1];
    y[2] = x[0] - x[stride];
    y[3] = x[1] - x[stride + 1];
    return;
  }
  else if (n == 1)
  {
    y[0] = x[0];
    y[1] = x[1];
    return;
  }
#endif

  // Recursion -- Decimation In Time algorithm
  split_radix_fft(x, y, n / 2, 2 * stride, twiddle_factors, 2 * tw_stride);
  split_radix_fft(x + stride, y + n, n / 4, 4 * stride, twiddle_factors, 4 * tw_stride);
  split_radix_fft(x + 3 * stride, y + n + n / 2, n / 4, 4 * stride, twiddle_factors, 4 * tw_stride);

  // Stitch together the output
  float u1r, u1i, u2r, u2i, x1r, x1i, x2r, x2i;
  float t;

  // We can save a few multiplications in the first step
  u1r = y[0];
  u1i = y[1];
  u2r = y[n / 2];
  u2i = y[n / 2 + 1];

  x1r = y[n];
  x1i = y[n + 1];
  x2r = y[n / 2 + n];
  x2i = y[n / 2 + n + 1];

  t = x1r + x2r;
  y[0] = u1r + t;
  y[n]     = u1r - t;

  t = x1i + x2i;
  y[1] = u1i + t;
  y[n + 1] = u1i - t;

  t = x2i - x1i;
  y[n / 2]     = u2r - t;
  y[n + n / 2]     = u2r + t;

  t = x1r - x2r;
  y[n / 2 + 1] = u2i - t;
  y[n + n / 2 + 1] = u2i + t;

  for (k = 1 ; k < n / 4 ; k++)
  {
    float u1r, u1i, u2r, u2i, x1r, x1i, x2r, x2i, c1, s1, c2, s2;
    c1 = twiddle_factors[k * tw_stride];
    s1 = twiddle_factors[k * tw_stride + 1];
    c2 = twiddle_factors[3 * k * tw_stride];
    s2 = twiddle_factors[3 * k * tw_stride + 1];

    u1r = y[2 * k];
    u1i = y[2 * k + 1];
    u2r = y[2 * k + n / 2];
    u2i = y[2 * k + n / 2 + 1];

    x1r =  c1 * y[n + 2 * k] + s1 * y[n + 2 * k + 1];
    x1i = -s1 * y[n + 2 * k] + c1 * y[n + 2 * k + 1];
    x2r =  c2 * y[n / 2 + n + 2 * k] + s2 * y[n / 2 + n + 2 * k + 1];
    x2i = -s2 * y[n / 2 + n + 2 * k] + c2 * y[n / 2 + n + 2 * k + 1];

    t = x1r + x2r;
    y[2 * k]     = u1r + t;
    y[2 * k + n]     = u1r - t;

    t = x1i + x2i;
    y[2 * k + 1] = u1i + t;
    y[2 * k + n + 1] = u1i - t;

    t = x2i - x1i;
    y[2 * k + n / 2]     = u2r - t;
    y[2 * k + n + n / 2]     = u2r + t;

    t = x1r - x2r;
    y[2 * k + n / 2 + 1] = u2i - t;
    y[2 * k + n + n / 2 + 1] = u2i + t;
  }

}


void ifft_primitive(float *input, float *output, int n, int stride, float *twiddle_factors, int tw_stride)
{

#if USE_SPLIT_RADIX
  split_radix_fft(input, output, n, stride, twiddle_factors, tw_stride);
#else
  fft_primitive(input, output, n, stride, twiddle_factors, tw_stride);
#endif

  int ks;

  int ns = n * stride;

  // reverse all coefficients from 1 to n / 2 - 1
  for (ks = stride ; ks < ns / 2 ; ks += stride)
  {
    float t;

    t = output[ks];
    output[ks] = output[ns-ks];
    output[ns-ks] = t;

    t = output[ks+1];
    output[ks+1] = output[ns-ks+1];
    output[ns-ks+1] = t;
  }

  // Apply normalization
  float norm = 1. / n;
  for (ks = 0 ; ks < ns ; ks += stride)
  {
    output[ks]   *= norm;
    output[ks+1] *= norm;
  }

}

inline void fft8(float *input, int stride_in, float *output, int stride_out)
{
  /*
   * Unrolled implementation of FFT8 for a little more performance
   */
  float a0r, a1r, a2r, a3r, a4r, a5r, a6r, a7r;
  float a0i, a1i, a2i, a3i, a4i, a5i, a6i, a7i;
  float b0r, b1r, b2r, b3r, b4r, b5r, b6r, b7r;
  float b0i, b1i, b2i, b3i, b4i, b5i, b6i, b7i;
  float t;
  float sin_pi_4 = 0.7071067812;

  a0r = input[0];
  a0i = input[1];
  a1r = input[stride_in];
  a1i = input[stride_in+1];
  a2r = input[2*stride_in];
  a2i = input[2*stride_in+1];
  a3r = input[3*stride_in];
  a3i = input[3*stride_in+1];
  a4r = input[4*stride_in];
  a4i = input[4*stride_in+1];
  a5r = input[5*stride_in];
  a5i = input[5*stride_in+1];
  a6r = input[6*stride_in];
  a6i = input[6*stride_in+1];
  a7r = input[7*stride_in];
  a7i = input[7*stride_in+1];

  // Stage 1

  b0r = a0r + a4r;
  b0i = a0i + a4i;

  b1r = a1r + a5r;
  b1i = a1i + a5i;

  b2r = a2r + a6r;
  b2i = a2i + a6i;

  b3r = a3r + a7r;
  b3i = a3i + a7i;

  b4r = a0r - a4r;
  b4i = a0i - a4i;

  b5r = a1r - a5r;
  b5i = a1i - a5i;
  // W_8^1 = 1/sqrt(2) - j / sqrt(2)
  t = b5r + b5i;
  b5i = (b5i - b5r) * sin_pi_4;
  b5r = t * sin_pi_4;

  // W_8^2 = -j
  b6r = a2i - a6i;
  b6i = a6r - a2r;

  b7r = a3r - a7r;
  b7i = a3i - a7i;
  // W_8^3 = -1 / sqrt(2) + j / sqrt(2)
  t = sin_pi_4 * (b7i - b7r);
  b7i = - (b7r + b7i) * sin_pi_4;
  b7r = t;

  // Stage 2

  a0r = b0r + b2r;
  a0i = b0i + b2i;

  a1r = b1r + b3r;
  a1i = b1i + b3i;

  a2r = b0r - b2r;
  a2i = b0i - b2i;

  // * j
  a3r = b1i - b3i;
  a3i = b3r - b1r;

  a4r = b4r + b6r;
  a4i = b4i + b6i;

  a5r = b5r + b7r;
  a5i = b5i + b7i;

  a6r = b4r - b6r;
  a6i = b4i - b6i;

  // * j
  a7r = b5i - b7i;
  a7i = b7r - b5r;

  // Stage 3

  // X[0]
  output[0] = a0r + a1r;
  output[1] = a0i + a1i;

  // X[4]
  output[4*stride_out] = a0r - a1r;
  output[4*stride_out+1] = a0i - a1i;

  // X[2]
  output[2*stride_out] = a2r + a3r;
  output[2*stride_out+1] = a2i + a3i;

  // X[6]
  output[6*stride_out] = a2r - a3r;
  output[6*stride_out+1] = a2i - a3i;

  // X[1]
  output[stride_out] = a4r + a5r;
  output[stride_out+1] = a4i + a5i;

  // X[5]
  output[5*stride_out] = a4r - a5r;
  output[5*stride_out+1] = a4i - a5i;

  // X[3]
  output[3*stride_out] = a6r + a7r;
  output[3*stride_out+1] = a6i + a7i;

  // X[7]
  output[7*stride_out] = a6r - a7r;
  output[7*stride_out+1] = a6i - a7i;

}

inline void fft4(float *input, int stride_in, float *output, int stride_out)
{
  /*
   * Unrolled implementation of FFT4 for a little more performance
   */
  float t1, t2;

  t1 = input[0] + input[2*stride_in];
  t2 = input[stride_in] + input[3*stride_in];
  output[0] = t1 + t2;
  output[2*stride_out] = t1 - t2;

  t1 = input[1] + input[2*stride_in+1];
  t2 = input[stride_in+1] + input[3*stride_in+1];
  output[1] = t1 + t2;
  output[2*stride_out+1] = t1 - t2;

  t1 = input[0] - input[2*stride_in];
  t2 = input[stride_in+1] - input[3*stride_in+1];
  output[stride_out] = t1 + t2;
  output[3*stride_out] = t1 - t2;

  t1 = input[1] - input[2*stride_in+1];
  t2 = input[3*stride_in] - input[stride_in];
  output[stride_out+1] = t1 + t2;
  output[3*stride_out+1] = t1 - t2;
}
/**
 * 范围映射
 * @param x
 * @param in_min
 * @param in_max
 * @param out_min
 * @param out_max
 * @return
 */
int map(float x,int in_min,int in_max,int out_min,int out_max){
    return (x-in_min)*(out_max-out_min)/(in_max-in_min)+out_min;
}
/**
 * 约束范围
 * @param x
 * @param a
 * @param b
 * @return
 */
int constrain(int x,int a,int b){
 if(x<a) return a;
 if(x>b) return b;
 return x;
}
/**
 * 直接输出振幅结果，需要一些范围映射参数
 * @param n_args
 * @param args
 * @return
 */
static mp_obj_t lvgl_esp32_FFT_execute_fit_win(size_t n_args, const mp_obj_t *args) {
    ESP_LOGI(TAG,"FFT execute");
    lvgl_esp32_FFT_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    ESP_LOGI(TAG,"FFT Get Pointer");
    if(n_args!=5){
        mp_raise_ValueError(MP_ERROR_TEXT("invalid arguments length:input,map_start,map_end,win_height"));
    }

    size_t len;
    mp_obj_t *items;
    mp_obj_get_array(args[1], &len, &items);
    ESP_LOGI(TAG,"FFT Get Input len:%d",len);
    if(len!=self->config->size){
        mp_raise_ValueError(MP_ERROR_TEXT("invalid len"));
    }
    int range_start=mp_obj_get_int(args[2]);
    int range_end=mp_obj_get_int(args[3]);

    int height=mp_obj_get_int(args[4]);
    ESP_LOGI(TAG,"FFT COPY");
    // 复制输入数据
    for (size_t i = 0; i < len; i++) {
        self->config->input[i] = map(mp_obj_get_float(items[i]),INT16_MIN,INT16_MAX,range_start,range_end);
    }
    ESP_LOGI(TAG,"FFT INPUT");

    fft_execute(self->config);
    ESP_LOGI(TAG,"FFT EXEC");
    // 创建返回列表
    mp_obj_t result = mp_obj_new_list(len/2, NULL);
    for (size_t i = 1; i <len/2; i++) {
        double magnitude=sqrt(pow(self->config->output[2*i],2)+pow(self->config->output[2*i+1],2));
        magnitude= constrain(magnitude,0,range_end);
        magnitude=map(magnitude,0,range_end,0,height);
        mp_obj_list_store(result, MP_OBJ_NEW_SMALL_INT(i), mp_obj_new_int(magnitude));
    }
    ESP_LOGI(TAG,"FFT OK");
    return result;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(lvgl_esp32_FFT_execute_fit_win_obj, 5, 5, lvgl_esp32_FFT_execute_fit_win);

static mp_obj_t lvgl_esp32_FFT_execute(mp_obj_t self_ptr, mp_obj_t input_list) {
    ESP_LOGI(TAG,"FFT execute");
    lvgl_esp32_FFT_obj_t *self = MP_OBJ_TO_PTR(self_ptr);
    ESP_LOGD(TAG,"FFT Get Pointer");

    size_t len;
    mp_obj_t *items;
    mp_obj_get_array(input_list, &len, &items);
    ESP_LOGD(TAG,"FFT Get Input len:%d",len);
    if(len!=self->config->size){
        mp_raise_ValueError(MP_ERROR_TEXT("invalid len"));
    }
    ESP_LOGD(TAG,"FFT COPY");
    // 复制输入数据
    for (size_t i = 0; i < len; i++) {
        self->config->input[i] = mp_obj_get_float(items[i]);
    }
    ESP_LOGD(TAG,"FFT INPUT");

    fft_execute(self->config);
    ESP_LOGD(TAG,"FFT EXEC");

    // 创建返回列表
    mp_obj_t result = mp_obj_new_list(len, NULL);
    for (size_t i = 0; i <len; i++) {
        mp_obj_list_store(result, MP_OBJ_NEW_SMALL_INT(i), mp_obj_new_float(self->config->output[i]));
    }
    ESP_LOGD(TAG,"FFT OK");
    return result;
}
static MP_DEFINE_CONST_FUN_OBJ_2(lvgl_esp32_FFT_execute_obj, lvgl_esp32_FFT_execute);

/*
static mp_obj_t lvgl_esp32_FFT_init(mp_obj_t self_ptr)
{
    lvgl_esp32_FFT_obj_t *self = MP_OBJ_TO_PTR(self_ptr);
    self->config=fft_init(self->size,self->fft_type,self->direction,NULL,NULL);

    ESP_LOGI(TAG,"Initializing FFT");
    return mp_obj_new_int_from_uint(0);
}
static MP_DEFINE_CONST_FUN_OBJ_1(lvgl_esp32_FFT_init_obj, lvgl_esp32_FFT_init);
*/

static mp_obj_t lvgl_esp32_FFT_deinit(mp_obj_t self_ptr)
{
    lvgl_esp32_FFT_obj_t *self = MP_OBJ_TO_PTR(self_ptr);
    if(self->config){
        fft_destroy(self->config);
    }
    ESP_LOGI(TAG, "Destroy FFT");
    return mp_obj_new_int_from_uint(0);
}
static MP_DEFINE_CONST_FUN_OBJ_1(lvgl_esp32_FFT_deinit_obj, lvgl_esp32_FFT_deinit);

static mp_obj_t lvgl_esp32_FFT_make_new(
        const mp_obj_type_t *type,
        size_t n_args,
        size_t n_kw,
        const mp_obj_t *all_args
)
{
    enum
    {
        ARG_size,
        ARG_fftType,
        ARG_direction,

    };

    static const mp_arg_t allowed_args[] = {
            { MP_QSTR_size, MP_ARG_INT | MP_ARG_REQUIRED },
            { MP_QSTR_fftType, MP_ARG_INT | MP_ARG_REQUIRED },
            { MP_QSTR_direction, MP_ARG_INT | MP_ARG_REQUIRED },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    lvgl_esp32_FFT_obj_t *self = mp_obj_malloc_with_finaliser(lvgl_esp32_FFT_obj_t, &lvgl_esp32_FFT_type);

    int size = args[ARG_size].u_int;
    int fft_type = args[ARG_fftType].u_int;
    int direction= args[ARG_direction].u_int;
    if(fft_type !=FFT_REAL && fft_type!=FFT_COMPLEX){
        mp_raise_ValueError(MP_ERROR_TEXT("invalid fft type"));
    }
    if(direction !=FFT_FORWARD && direction!=FFT_BACKWARD){
        mp_raise_ValueError(MP_ERROR_TEXT("invalid fft direction"));
    }
    self->size=size;
    self->fft_type=fft_type;
    self->direction=direction;
    self->config=fft_init(self->size,self->fft_type,self->direction,NULL,NULL);
    return MP_OBJ_FROM_PTR(self);
}
static const mp_rom_map_elem_t lvgl_esp32_FFT_locals_table[] = {
        { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&lvgl_esp32_FFT_deinit_obj) },
        { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&lvgl_esp32_FFT_deinit_obj) },
        { MP_ROM_QSTR(MP_QSTR_execute), MP_ROM_PTR(&lvgl_esp32_FFT_execute_obj) },
        { MP_ROM_QSTR(MP_QSTR_execute_fit_win), MP_ROM_PTR(&lvgl_esp32_FFT_execute_fit_win_obj) },
        //常量
        { MP_ROM_QSTR(MP_QSTR_REAL),          MP_ROM_INT(FFT_REAL) },
        { MP_ROM_QSTR(MP_QSTR_COMPLEX),       MP_ROM_INT(FFT_COMPLEX) },
        //常量
        { MP_ROM_QSTR(MP_QSTR_FORWARD),          MP_ROM_INT(FFT_FORWARD) },
        { MP_ROM_QSTR(MP_QSTR_BACKWARD),       MP_ROM_INT(FFT_BACKWARD) },

};

static MP_DEFINE_CONST_DICT(lvgl_esp32_FFT_locals, lvgl_esp32_FFT_locals_table);

MP_DEFINE_CONST_OBJ_TYPE(
        lvgl_esp32_FFT_type,
        MP_QSTR_FFT,
        MP_TYPE_FLAG_NONE,
        make_new,
        lvgl_esp32_FFT_make_new,
        locals_dict,
&lvgl_esp32_FFT_locals
);