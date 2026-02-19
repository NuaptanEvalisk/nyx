//
// Created by felix on 2/17/26.
//

#ifndef NYX_NMATH_H
#define NYX_NMATH_H

#include "nmath_err_list.h"
#include "nmath_types.h"

#include <stddef.h> // NOLINT(*-deprecated-headers)
#include <stdint.h> // NOLINT(*-deprecated-headers)

#if defined(__cplusplus)
extern "C"
{
#endif

  enum NMath_Error_Info nmath_render_fast(const char *formula,
                                          uint32_t *pixels,
                                          nmath_geometry_t *geometry,
                                          size_t buf_size,
                                          nmath_runtime_t runtime);

  enum NMath_Error_Info nmath_render_quality(const char *formula,
                                             enum NMath_Formula_Type ft,
                                             nmath_bitmap_t *bitmap,
                                             nmath_runtime_t runtime);

  enum NMath_Error_Info nmath_lualatex_precompile(nmath_runtime_t runtime);

  enum NMath_Error_Info nmath_initialize_microtex(nmath_runtime_t runtime);

  enum NMath_Error_Info nmath_release_microtex(void);

  enum NMath_Error_Info
  nmath_runtime_create(const nmath_global_config_t *globals,
                       nmath_runtime_t *rt);

  enum NMath_Error_Info nmath_runtime_reset(
    const nmath_global_config_t *globals, nmath_runtime_t runtime);

  enum NMath_Error_Info nmath_runtime_destroy(nmath_runtime_t runtime);

#if defined(__cplusplus)
}
#endif

#endif // NYX_NMATH_H
