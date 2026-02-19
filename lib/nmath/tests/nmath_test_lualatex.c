//
// Created by felix on 2/19/26.
//

#include "nmath_test_internals.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(void)
{
  setbuf(stdout, NULL);
  printf("NMath Test Program, LuaLaTeX Rendering.\n");

  struct timespec t0, t1;

  clock_gettime(CLOCK_MONOTONIC, &t0);
  nmath_runtime_t runtime = NULL;
  nmath_global_config_t globals;
  globals.lualatex_exe_path = "/usr/bin/pdflatex";
  globals.lualatex_shm_dir = "/dev/shm/nyx/";
  system("mkdir -p /dev/shm/nyx/");
  globals.lualatex_job_name = "nmath-demo-job";
  globals.tex_fmt_source_path = "../../assets/latex/nmath_latex_fmt.tex";
  globals.tex_head_path = "../../assets/latex/nmath_latex_head.tex";
  globals.tex_tail_path = "../../assets/latex/nmath_latex_tail.tex";
  globals.lualatex_dump_path = "/dev/shm/nyx/preload.fmt";
  globals.pdf_scale_factor = 3.0f;
  enum NMath_Error_Info e = nmath_runtime_create(&globals, &runtime);
  err_proc(e);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Created NMath runtime at %p. [%.2f ms]\n", (void *)runtime,
         get_ms(t0, t1));

  clock_gettime(CLOCK_MONOTONIC, &t0);
  e = nmath_lualatex_precompile(runtime);
  err_proc(e);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Precompiled LuaLaTeX prelude. [%.2f ms]\n", get_ms(t0, t1));

  const char *formula = "\\sum\\prod_{i=1}^n X_i=\\bigvee_{\\emptyset"
                      "\\subsetneq\\left\\{i_1,\\cdots,i_\\ell\\right\\}"
                      "\\subset\\left\\{1,\\cdots,n\\right\\}}\\sum"
                      "\\bigwedge_{i_j}X_{i_j}";
  nmath_geometry_t geo;
  nmath_bitmap_t bitmap;
  bitmap.pixels = malloc(200 * 200 * 4);
  assert(bitmap.pixels != NULL);
  memset(bitmap.pixels, 0, 200 * 200 * 4);

  clock_gettime(CLOCK_MONOTONIC, &t0);
  e = nmath_render_quality(formula, nmath_formula_display, &bitmap, &geo,
                           runtime);
  err_proc(e);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Rendered NMath formula using LuaLaTeX backend. [%.2f ms]\n",
         get_ms(t0, t1));
  printf("Formula geomtry: width = %d, height = %d, baseline = %.2f\n",
         geo.width, geo.height, geo.baseline);

  clock_gettime(CLOCK_MONOTONIC, &t0);
  stbi_write_png("nmath.tex.png",
    bitmap.width, bitmap.height, 4, bitmap.pixels, bitmap.width * 4);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Exported to nmath.tex.png [%.2f ms]\n", get_ms(t0, t1));

  clock_gettime(CLOCK_MONOTONIC, &t0);
  free(bitmap.pixels);
  e = nmath_runtime_destroy(runtime);
  err_proc(e);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Destroyed NMath runtime. [%.2f ms]\n", get_ms(t0, t1));

  return EXIT_SUCCESS;
}