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

struct nmath_runtime_internal
{
  nmath_global_config_t globals;
  sk_sp<SkFontMgr> font_mgr;
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
