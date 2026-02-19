//
// Created by felix on 2/19/26.
//

#include "nmath_internals.hpp"
#include "graphic/graphic_basic.h"
#include <codecvt>
#include <include/core/SkImageInfo.h>
#include <include/core/SkSurface.h>
#include <latex.h>

enum NMath_Error_Info nmath_render_fast(const char *formula,
                                        uint32_t *pixels,
                                        nmath_geometry_t *geometry,
                                        const size_t buf_size,
                                        nmath_runtime_t runtime)
{
  (void)buf_size; // will add check some time later.

  if (formula == nullptr || pixels == nullptr || runtime == nullptr)
  {
    return nmath_err_null_check;
  }

  NMathMicrotexContextGuard guard(runtime->font_mgr.get());

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
  // ReSharper disable once CppDeprecatedEntity
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  const std::wstring f = converter.from_bytes(formula);
#pragma clang diagnostic pop

  tex::TeXRender *r =
      tex::LaTeX::parse(f, runtime->globals.width_allowed,
                        runtime->globals.font_size,
                        runtime->globals.line_spacing, tex::black);

  geometry->width = r->getWidth();
  geometry->height = r->getHeight();
  geometry->baseline = r->getBaseline();

  const SkImageInfo info =
    SkImageInfo::Make(geometry->width, geometry->height,
                   kRGBA_8888_SkColorType,
                   kPremul_SkAlphaType);
  sk_sp<SkSurface> surface = SkSurfaces::WrapPixels(info, pixels, info.minRowBytes());
  if (!surface)
  {
    return nmath_err_wrap_pixels_failure;
  }

  SkCanvas *canvas = surface->getCanvas();
  if (!canvas)
  {
    return nmath_err_wrap_pixels_failure;
  }

  const auto g2 = create_skia_graphics(canvas);
  r->draw(*g2, 0, 0);
  delete r;

  return nmath_okay;
}