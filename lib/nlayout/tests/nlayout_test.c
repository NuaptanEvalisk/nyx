//
// Created by felix on 2/16/26.
//

#include "nlayout/nlayout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FONT_DIR ASSETS_DIR "/fonts/"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void err_proc(enum NLayout_Error_Info e)
{
  if (e != nlayout_okay)
  {
    const char *estr = nlayout_explain_err(e);
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

int main (void)
{
  printf("NLayout Test Program.\n");

  struct timespec t0, t1;

  nlayout_buffer_t out;
  out.pixels = malloc(2000 * 2000 * 4);
  memset(out.pixels, 0, 2000 * 2000 * 4);

  clock_gettime(CLOCK_MONOTONIC, &t0);
  nlayout_runtime_t runtime = NULL;
  nlayout_global_config_t globals;
  globals.width = 1920;
  globals.height = 1080;
  globals.padding_left = 10;
  globals.padding_right = 10;
  globals.scan_os_fonts = false;
  globals.wrap_pixels = true;
  enum NLayout_Error_Info e = nlayout_runtime_create(&globals, &runtime, out.pixels);
  err_proc(e);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Created NLayout runtime at %p. [%.2f ms]\n", (void*)runtime, get_ms(t0, t1));
  fflush(stdout);

  clock_gettime(CLOCK_MONOTONIC, &t0);
  char *font_files[] = {
    FONT_DIR "LibertinusSerif-BoldItalic.ttf",
    FONT_DIR "LibertinusSerif-Bold.ttf",
    FONT_DIR "LibertinusSerif-Italic.ttf",
    FONT_DIR "LibertinusSerif-Regular.ttf"
  };
  const nlayout_font_entry_t libertinus = {
      "Libertinus Serif", strlen("Libertinus Serif"),
      nlayout_font_bold, 40, font_files, 4 };
  e = nlayout_runtime_reg_font(&libertinus, runtime);
  err_proc(e);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Registered font Libertinus Serif to runtime. [%.2f ms]\n", get_ms(t0, t1));
  fflush(stdout);

  clock_gettime(CLOCK_MONOTONIC, &t0);
  const char lorem_ipsum[] = "Lorem ipsum dolor sit amet, consectetur "
                             "adipiscing elit, sed do eiusmod tempor incididunt"
                             " ut labore et dolore magna aliqua. Ut enim ad "
                             "minim veniam, quis nostrud exercitation ullamco "
                             "laboris nisi ut aliquip ex ea commodo consequat. "
                             "Duis aute irure dolor in reprehenderit in"
                             " voluptate velit esse cillum dolore eu fugiat "
                             "nulla pariatur. Excepteur sint occaecat cupidatat"
                             " non proident, sunt in culpa qui officia deserunt"
                             " mollit anim id est laborum.";
  nlayout_instruction_t i1;
  i1.offset = 0;
  i1.asset_type = nlayout_asset_text_style;
  nlayout_text_style_t s1;
  s1.bg_color.red = 0, s1.bg_color.green = 0, s1.bg_color.blue = 120;
  s1.bg_color.alpha = 30;
  s1.fg_color.red = 255, s1.fg_color.green = 255, s1.fg_color.blue = 0;
  s1.fg_color.alpha = 255;
  s1.font_id = 0;
  i1.asset.text_style = s1;

  nlayout_instruction_t i2;
  i2.offset = 5;
  i2.asset_type = nlayout_asset_inline_box;
  i2.asset.box.width = 200;
  i2.asset.box.height = 10;
  i2.asset.box.baseline_shift = 0;

  nlayout_instruction_t i3;
  i3.offset = 15;
  i3.asset_type = nlayout_asset_display_box;
  i3.asset.box.width = 300;
  i3.asset.box.height = 200;
  i3.asset.box.baseline_shift = 0;

  nlayout_instruction_t i4;
  i4.offset = 20;
  i4.asset_type = nlayout_asset_cursor;

  nlayout_instruction_t i5;
  i5.offset = 25;
  i5.asset_type = nlayout_asset_container_style;
  i5.asset.con_style.bg_color.red = 0, i5.asset.con_style.bg_color.green = 100;
  i5.asset.con_style.bg_color.blue = 0, i5.asset.con_style.bg_color.alpha = 200;
  i5.asset.con_style.padding_left = 200, i5.asset.con_style.padding_right = 100;
  i5.asset.con_style.padding_bottom = 4, i5.asset.con_style.padding_top = 6;
  nlayout_color_t bc;
  bc.alpha = 0, bc.red = 255, bc.green = 255, bc.blue = 255;
  i5.asset.con_style.border_color = bc;
  i5.asset.con_style.border_thickness = 5;
  i5.asset.con_style.s_override_heading.count = 0;
  i5.asset.con_style.s_override_body.count = 0;

  nlayout_instruction_t i6;
  i6.offset = 25;
  i6.asset_type = nlayout_asset_container_op;
  i6.asset.con_operation = nlayout_container_begin;

  nlayout_instruction_t i7;
  i7.offset = 25;
  i7.asset_type = nlayout_asset_container_op;
  i7.asset.con_operation = nlayout_container_body;

  nlayout_instruction_t i8;
  i8.offset = 180;
  i8.asset_type = nlayout_asset_container_op;
  i8.asset.con_operation = nlayout_container_end;

  nlayout_instruction_t ins[] = { i1, i2, i3, i4, i5, i6, i7, i8 };
  e = nlayout_layout(runtime, lorem_ipsum, strlen(lorem_ipsum), ins, 8);
  err_proc(e);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Computed layout for lorem ipsum. [%.2f ms]\n", get_ms(t0, t1));
  fflush(stdout);

  clock_gettime(CLOCK_MONOTONIC, &t0);
  e = nlayout_compute_hits(runtime, strlen(lorem_ipsum), ins, 8);
  err_proc(e);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Computed hit testing info for lorem ipsum. [%.2f ms]\n", get_ms(t0, t1));
  fflush(stdout);

  // nlayout_iter_fill_box functions should be iterated now, but we will run it
  // in this test. let's leave the iboxes and dboxes blank for this time.

  clock_gettime(CLOCK_MONOTONIC, &t0);
  e = nlayout_export(runtime, 2000 * 2000 * 4, &out);
  err_proc(e);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Exported pixels to %p, %d x %d, stride = %d. [%.2f ms]\n", &out.pixels,
    out.width, out.height, out.stride, get_ms(t0, t1));
  fflush(stdout);

  clock_gettime(CLOCK_MONOTONIC, &t0);
  stbi_write_png("demo.png",
    out.width, out.height, 4, out.pixels, out.stride);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Exported to demo.png [%.2f ms]\n", get_ms(t0, t1));
  fflush(stdout);

  clock_gettime(CLOCK_MONOTONIC, &t0);
  free(out.pixels);
  e = nlayout_runtime_destroy(runtime);
  err_proc(e);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  printf("Destroyed NLayout runtime. [%.2f ms]\n", get_ms(t0, t1));
  fflush(stdout);

  return EXIT_SUCCESS;
}