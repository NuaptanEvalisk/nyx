//
// Created by felix on 2/14/26.
//

#include "nlayout_internals.hpp"
#include "include/ports/SkFontMgr_empty.h"

NLayout_Error_Info nlayout_runtime_create(const nlayout_global_config_t *globals,
                            nlayout_runtime_t *rt, uint32_t *buffer)
{
  if (rt == nullptr)
  {
    return nlayout_err_null_check;
  }
  if (*rt == nullptr)
  {
    *rt = static_cast<nlayout_runtime_t>(
      malloc(sizeof(nlayout_runtime_internal)));
    if (*rt == nullptr)
    {
      return nlayout_err_failed_alloc;
    }
  }
  nlayout_runtime_t runtime = *rt;

  if (globals->width <= 0 || globals->height <= 0)
  {
    return nlayout_err_invalid_dimensions;
  }

  runtime->globals = *globals;
  runtime->box_data_count = 0;
  runtime->box_data_count_backup = 0;

  runtime->box_data = static_cast<nlayout_box_data_t *>(
      malloc(sizeof(nlayout_box_data_t) * NLAYOUT_BOX_ALLOWED));
  if (runtime->box_data == nullptr)
  {
    return nlayout_err_failed_alloc;
  }
  memset(runtime->box_data, 0,
    sizeof(nlayout_box_data_t) * NLAYOUT_BOX_ALLOWED);

  runtime->cursor = (nlayout_cursor_t) { -1, -1, -1 };
  runtime->fonts.count = 0;
  runtime->fonts.fonts = static_cast<nlayout_font_entry_t *>(
    malloc(sizeof(nlayout_font_entry_t) * NLAYOUT_FONTS_ALLOWED));
  if (runtime->fonts.fonts == nullptr)
  {
    return nlayout_err_failed_alloc;
  }
  memset(runtime->fonts.fonts, 0,
    sizeof(nlayout_font_entry_t) * NLAYOUT_FONTS_ALLOWED);

  runtime->font_collection = sk_make_sp<skia::textlayout::FontCollection>();
  auto scanner = SkFontScanner_Make_FreeType();
  if (globals->scan_os_fonts)
  {
    runtime->font_mgr = SkFontMgr_New_FontConfig(nullptr, std::move(scanner));
  }
  else
  {
    runtime->font_mgr = SkFontMgr_New_Custom_Empty();
  }
  runtime->font_provider = sk_make_sp<skia::textlayout::TypefaceFontProvider>();
  runtime->font_collection->setAssetFontManager(runtime->font_provider);
  runtime->font_collection->setDefaultFontManager(runtime->font_mgr);

  const SkImageInfo info =
      SkImageInfo::Make(globals->width, globals->height,
                     kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType);

  if (buffer == nullptr || globals->wrap_pixels == false)
  {
    runtime->surface = SkSurfaces::Raster(info);
    if (runtime->surface == nullptr)
    {
      return nlayout_err_failed_alloc;
    }
  }
  else
  {
    debug_info_msg("Buffer at " << buffer << " is available.");
    debug_info_msg("info.minRowBytes() = " << info.minRowBytes());

    runtime->surface = SkSurfaces::WrapPixels(info, buffer, info.minRowBytes());
    if (!runtime->surface)
    {
      return nlayout_err_failed_to_wrap_pixels;
    }
  }

  runtime->canvas = runtime->surface->getCanvas();

  runtime->unicode = SkUnicodes::ICU::Make();
  if (runtime->unicode == nullptr)
  {
    return nlayout_err_invalid_icu_data;
  }

  runtime->ht_lanes.clear();

  return nlayout_okay;
}

NLayout_Error_Info nlayout_runtime_reg_font(const nlayout_font_entry_t *font,
                                nlayout_runtime_t runtime)
{
  if (font == nullptr || font->font_files == nullptr || *font->font_files == nullptr)
  {
    return nlayout_err_null_check;
  }

  for (uint8_t i = 0; i < font->font_files_count; ++i)
  {
    const auto fontData = SkData::MakeFromFileName(font->font_files[i]);
    if (fontData == nullptr)
    {
      debug_err_msg("Failed to create SkData: " << font->font_files[i]);
      return nlayout_err_invalid_font;
    }
    const auto tf = runtime->font_mgr->makeFromData(fontData);
    if (tf == nullptr)
    {
      debug_err_msg("Failed to make font manager from data: " << font->font_files[i]);
      return nlayout_err_invalid_font;
    }
    runtime->font_provider->registerTypeface(tf, SkString(font->name));
  }
  debug_info_msg("Registered all font files to font provider.");

  runtime->fonts.fonts[runtime->fonts.count] = *font;
  ++runtime->fonts.count;

  return nlayout_okay;
}

NLayout_Error_Info nlayout_runtime_reset(const nlayout_global_config_t *globals,
  nlayout_runtime_t runtime)
{
  if (runtime == nullptr)
  {
    return nlayout_err_null_check;
  }
  if (globals->width <= 0 || globals->height <= 0)
  {
    return nlayout_err_invalid_dimensions;
  }

  runtime->globals = *globals;
  runtime->box_data_count = 0;
  runtime->box_data_count_backup = 0;
  if (runtime->box_data == nullptr)
  {
    return nlayout_err_failed_alloc;
  }
  memset(runtime->box_data, 0,
    sizeof(nlayout_box_data_t) * NLAYOUT_BOX_ALLOWED);

  runtime->cursor = (nlayout_cursor_t) { -1, -1, -1 };

  const SkImageInfo info = SkImageInfo::MakeN32Premul(globals->width, globals->height);
  runtime->surface = SkSurfaces::Raster(info);
  if (runtime->surface == nullptr)
  {
    return nlayout_err_failed_alloc;
  }
  runtime->canvas = runtime->surface->getCanvas();

  runtime->ht_lanes.clear();

  return nlayout_okay;
}

NLayout_Error_Info nlayout_runtime_destroy(nlayout_runtime_t runtime)
{
  runtime->surface.reset();
  runtime->font_collection.reset();
  runtime->font_mgr.reset();
  runtime->font_provider.reset();
  runtime->unicode.reset();
  runtime->canvas = nullptr;
  runtime->box_data_count = 0;
  runtime->fonts.count = 0;
  free(runtime->box_data);
  free(runtime->fonts.fonts);
  return nlayout_okay;
}