//
// Created by felix on 2/17/26.
//

#ifndef NYX_NMATH_DATA_H
#define NYX_NMATH_DATA_H

#if !defined(__cplusplus)
#include <stdbool.h>
#endif

#include <stdint.h> // NOLINT(*-deprecated-headers)

#if defined(__cplusplus)
extern "C"
{
#endif

  typedef struct
  {
    char *lualatex_exe_path;
    char *tex_head_path;
    char *tex_tail_path;

    char *tex_fmt_source_path;
    char *lualatex_dump_path;

    char *lualatex_shm_dir;
    char *lualatex_job_name;
    float pdf_scale_factor;

    bool scan_system_fonts;
    float font_size;
    float line_spacing;
    int width_allowed;
    char *microtex_res_path;
  } nmath_global_config_t;

  typedef struct
  {
    int width;
    int height;
    float baseline;
  } nmath_geometry_t;

  enum NMath_Formula_Type
  {
    nmath_formula_inline, nmath_formula_display
  };

  typedef struct
  {
    uint32_t *pixels;
    int width;
    int height;
    int stride;
    int data_size;
  } nmath_bitmap_t;

  // opaque pointer
  typedef struct nmath_runtime_internal *nmath_runtime_t;

#if defined(__cplusplus)
}
#endif


#endif // NYX_NMATH_DATA_H
