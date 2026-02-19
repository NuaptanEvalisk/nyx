//
// Created by felix on 2/18/26.
//

#ifndef NYX_NMATH_INTERNALS_HPP
#define NYX_NMATH_INTERNALS_HPP

#include "nmath/nmath.h"

#include <graphic/graphic.h>
#include <include/core/SkCanvas.h>
#include <include/core/SkFontMgr.h>
#include <include/core/SkRefCnt.h>

#define NMATH_DEBUG_OUTPUTS
#ifdef NMATH_DEBUG_OUTPUTS
#include <iostream>
#define debug_err_msg(msg) std::cerr << msg << std::endl << std::flush;
#define debug_info_msg(msg) std::cout << msg << std::endl << std::flush;
#else
#define debug_err_msg(msg)
#define debug_info_msg(msg)
#endif

struct nmath_runtime_internal
{
  nmath_global_config_t globals;
  sk_sp<SkFontMgr> font_mgr;

  std::string latex_head_content;
  std::string latex_tail_content;
};

extern thread_local SkFontMgr *g_tls_font_mgr;

class NMathMicrotexContextGuard
{
private:
  SkFontMgr *previous_mgr;

public:
  explicit NMathMicrotexContextGuard(SkFontMgr *mgr)
  {
    previous_mgr = g_tls_font_mgr;
    g_tls_font_mgr = mgr;
  }

  ~NMathMicrotexContextGuard() { g_tls_font_mgr = previous_mgr; }

  NMathMicrotexContextGuard(const NMathMicrotexContextGuard &) = delete;
  NMathMicrotexContextGuard &
  operator=(const NMathMicrotexContextGuard &) = delete;
};

// factory
std::unique_ptr<tex::Graphics2D> create_skia_graphics(SkCanvas* canvas);

#endif // NYX_NMATH_INTERNALS_HPP
