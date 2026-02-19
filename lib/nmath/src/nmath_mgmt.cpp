//
// Created by felix on 2/18/26.
//

#include "nmath_internals.hpp"
#include "include/core/SkFontScanner.h"
#include "include/ports/SkFontMgr_empty.h"

#include <include/ports/SkFontMgr_fontconfig.h>
#include <include/ports/SkFontScanner_FreeType.h>
#include <latex.h>

#include <fstream>
#include <string>

static bool read_file_content(const char* path, std::string& out_content)
{
  std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
  if (!file.is_open())
  {
    return false;
  }

  const std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  if (size <= 0)
  {
    return true;
  }

  out_content.resize(size);

  if (file.read(&out_content[0], size))
  {
    return true;
  }
  return false;
}

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
    debug_info_msg("Using system font scanner.");

    if (!runtime->font_mgr)
    {
      debug_err_msg("[Debug] FATAL: SkFontMgr_New_FontConfig returned nullptr");
    }
    else
    {
      const int familyCount = runtime->font_mgr->countFamilies();
      debug_info_msg("[Debug] FontMgr created successfully. It found " <<
        familyCount << " font families on the system.");
    }
  }
  else
  {
    runtime->font_mgr = SkFontMgr_New_Custom_Empty();
    debug_info_msg("Using empty font manager.");
  }

  const bool head_ok =
    read_file_content(globals->tex_head_path, runtime->latex_head_content);
  const bool tail_ok =
    read_file_content(globals->tex_tail_path, runtime->latex_tail_content);
  if (!(head_ok && tail_ok))
  {
    return nmath_err_cannot_read_parts;
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

  // if we do not mount thread_local variables here, would not find sys font.
  NMathMicrotexContextGuard guard(runtime->font_mgr.get());

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