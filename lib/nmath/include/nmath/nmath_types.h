//
// Created by felix on 2/17/26.
//

#ifndef NYX_NMATH_DATA_H
#define NYX_NMATH_DATA_H

#if !defined(__cplusplus)
#include <stdbool.h>
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

  typedef struct
  {
    char *lualatex_exe_path;
    char *tex_head_path;
    char *tex_tail_path;

    bool preload_lualatex;
    char *lualatex_dump_path;

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

  // opaque pointer
  typedef struct nmath_runtime_internal *nmath_runtime_t;

#if defined(__cplusplus)
}
#endif


#endif // NYX_NMATH_DATA_H
