//
// Created by felix on 2/18/26.
//

#include "nmath_internals.hpp"
#include "include/core/SkFontScanner.h"
#include "include/ports/SkFontMgr_empty.h"

#include <include/ports/SkFontMgr_fontconfig.h>
#include <include/ports/SkFontScanner_FreeType.h>
#include <latex.h>

enum NMath_Error_Info nmath_runtime_create(const nmath_global_config_t *globals,
                                           nmath_runtime_t *rt)
{
  if (rt == nullptr || globals == nullptr)
  {
    return nmath_err_null_check;
  }

  if (*rt == nullptr)
  {
    *rt = static_cast<nmath_runtime_t>(malloc(sizeof(nmath_runtime_internal)));
    if (*rt == nullptr)
    {
      return nmath_err_failed_alloc;
    }
  }

  nmath_runtime_t runtime = *rt;
  runtime->globals = *globals;

  if (globals->scan_system_fonts)
  {
    auto scanner = SkFontScanner_Make_FreeType();
    runtime->font_mgr = SkFontMgr_New_FontConfig(nullptr, std::move(scanner));
  }
  else
  {
    runtime->font_mgr = SkFontMgr_New_Custom_Empty();
  }

  return nmath_okay;
}

enum NMath_Error_Info nmath_runtime_destroy(nmath_runtime_t runtime)
{
  runtime->font_mgr.reset();
  return nmath_okay;
}

enum NMath_Error_Info nmath_initialize_microtex(nmath_runtime_t runtime)
{
  if (runtime == nullptr)
  {
    return nmath_err_null_check;
  }

  tex::LaTeX::init(runtime->globals.microtex_res_path);
  return nmath_okay;
}

enum NMath_Error_Info nmath_release_microtex(void)
{
  tex::LaTeX::release();
  return nmath_okay;
}

enum NMath_Error_Info nmath_runtime_reset(
  const nmath_global_config_t *globals, nmath_runtime_t runtime)
{
  if (runtime == nullptr || globals == nullptr)
  {
    return nmath_err_null_check;
  }
  runtime->globals = *globals;
  return nmath_okay;
}