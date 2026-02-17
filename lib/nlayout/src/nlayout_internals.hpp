//
// Created by felix on 2/14/26.
//

#ifndef NYX_NLAYOUT_INTERNALS_HPP
#define NYX_NLAYOUT_INTERNALS_HPP

#include "nlayout/nlayout.h"
#include "include/core/SkTypes.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkStream.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkFontScanner.h"
#include "include/ports/SkFontMgr_fontconfig.h"
#include "include/ports/SkFontScanner_FreeType.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkTypeface.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/include/SkUnicode_icu.h"
#include "modules/skshaper/include/SkShaper_harfbuzz.h"
#include "modules/skshaper/utils/FactoryHelpers.h"
#include <vector>

#define NLAYOUT_DEBUG_OUTPUTS
#ifdef NLAYOUT_DEBUG_OUTPUTS
#include <iostream>
#define debug_err_msg(msg) std::cerr << msg << std::endl << std::flush;
#define debug_info_msg(msg) std::cout << msg << std::endl << std::flush;
#else
#define debug_err_msg(msg)
#define debug_info_msg(msg)
#endif

// data structures for hit-testing
struct nlayout_ht_lane
{
  float top, left, right, bottom;
  std::vector<std::pair<nlayout_hit_t, float>> inline_map;
};

struct nlayout_runtime_internal
{
  nlayout_global_config_t globals;
  nlayout_font_registry_t fonts;
  nlayout_box_data_t *box_data;
  size_t box_data_count;
  nlayout_cursor_t cursor;

  size_t box_data_count_backup;

  sk_sp<SkSurface> surface;
  SkCanvas *canvas;

  sk_sp<skia::textlayout::FontCollection> font_collection;
  sk_sp<SkFontMgr> font_mgr;
  sk_sp<skia::textlayout::TypefaceFontProvider> font_provider;
  sk_sp<SkUnicode> unicode;

  std::vector<nlayout_ht_lane> ht_lanes;
};

#define NLAYOUT_BOX_ALLOWED 1000
#define NLAYOUT_FONTS_ALLOWED 100

NLayout_Error_Info nlayout_extract_placeholder(
  nlayout_runtime_t runtime,
  const nlayout_ins_asset_u *ins,
  skia::textlayout::PlaceholderStyle *ph);

NLayout_Error_Info nlayout_compile_text_style(
  nlayout_runtime_t runtime,
  nlayout_text_style_t text_style,
  skia::textlayout::TextStyle *style);

nlayout_text_style_t nlayout_apply_override(
  nlayout_text_style_t style,
  const nlayout_style_override_map_t *map);

NLayout_Error_Info nlayout_extract_text_style(
  nlayout_runtime_t runtime,
  const nlayout_ins_asset_u *ins,
  const nlayout_style_override_map_t *map,
  skia::textlayout::TextStyle *style);

NLayout_Error_Info nlayout_extract_overrides(
  nlayout_runtime_t runtime,
  const nlayout_instruction_t *ins,
  nlayout_style_override_map_t *h, nlayout_style_override_map_t *b);

NLayout_Error_Info nlayout_extract_con_geometry(
  nlayout_runtime_t runtime,
  const nlayout_instruction_t *ins,
  float *dx0, float *dx1, float *dy0, float *dy1);

#endif // NYX_NLAYOUT_INTERNALS_HPP
