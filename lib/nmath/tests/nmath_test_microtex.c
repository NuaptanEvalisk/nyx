//
// Created by felix on 2/19/26.
//

#include "nmath/nmath.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void err_proc(enum NMath_Error_Info e)
{
  if (e != nmath_okay)
  {
    const char *estr = nmath_explain_err(e);
    printf("NLayout error: %s\n", estr);
    fflush(stdout);
    exit(EXIT_FAILURE);
  }
}

double get_ms(const struct timespec start, const struct timespec end)
{
  return (double)(end.tv_sec - start.tv_sec) * 1000.0 +
         (double)(end.tv_nsec - start.tv_nsec) / 1000000.0;
}

int main(void)
{
  setbuf(stdout, NULL);
  printf("NMath Test Program, MicroTeX Rendering.\n");

  struct timespec t0, t1;

  clock_gettime(CLOCK_MONOTONIC, &t0);
  nmath_runtime_t runtime = NULL;
  nmath_global_config_t globals;
  globals.font_size = 25;
  globals.line_spacing = 1;
  globals.microtex_res_path = "../../res";
  globals.scan_system_fonts = true;
  globals.width_allowed = 200;
  enum NMath_Error_Info e = nmath_runtime_create(&globals, &runtime);
  err_proc(e);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Created NMath runtime at %p. [%.2f ms]\n", (void *)runtime,
         get_ms(t0, t1));

  clock_gettime(CLOCK_MONOTONIC, &t0);
  e = nmath_initialize_microtex(runtime);
  err_proc(e);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Initialized MicroTeX. [%.2f ms]\n", get_ms(t0, t1));

  const char *formula = "\\sum\\prod_{i=1}^n X_i=\\bigvee_{\\emptyset"
                        "\\subsetneq\\left\\{i_1,\\cdots,i_\\ell\\right\\}"
                        "\\subset\\left\\{1,\\cdots,n\\right\\}}\\sum"
                        "\\bigwedge_{i_j}X_{i_j}";
  nmath_geometry_t geo;
  uint32_t *pixels = malloc(200 * 200 * 4);
  assert(pixels != NULL);
  memset(pixels, 0, 200 * 200 * 4);

  clock_gettime(CLOCK_MONOTONIC, &t0);
  e = nmath_render_fast(formula, pixels, &geo, 200 * 200 * 4, runtime);
  err_proc(e);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Rendered NMath formula using MicroTeX backend. [%.2f ms]\n",
         get_ms(t0, t1));
  printf("Formula geomtry: width = %d, height = %d, baseline = %.2f\n",
         geo.width, geo.height, geo.baseline);

  clock_gettime(CLOCK_MONOTONIC, &t0);
  stbi_write_png("nmath.demo.png",
    geo.width, geo.height, 4, pixels, geo.width * 4);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Exported to nmath.demo.png [%.2f ms]\n", get_ms(t0, t1));

  clock_gettime(CLOCK_MONOTONIC, &t0);
  free(pixels);
  e = nmath_runtime_destroy(runtime);
  err_proc(e);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Destroyed NMath runtime. [%.2f ms]\n", get_ms(t0, t1));

  return EXIT_SUCCESS;
}