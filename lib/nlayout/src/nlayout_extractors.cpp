//
// Created by felix on 2/15/26.
//

#include "nlayout_internals.hpp"
#include <cassert>

NLayout_Error_Info nlayout_extract_placeholder(
  nlayout_runtime_t runtime,
  const nlayout_ins_asset_u *ins,
  skia::textlayout::PlaceholderStyle *ph)
{
  (void)runtime;

  if (ph == nullptr)
  {
    return nlayout_err_null_check;
  }
  ph->fWidth = ins->box.width;
  ph->fHeight = ins->box.height;
  ph->fAlignment = skia::textlayout::PlaceholderAlignment::kBaseline;
  ph->fBaselineOffset = ins->box.height;
  ph->fBaseline = skia::textlayout::TextBaseline::kAlphabetic;
  return nlayout_okay;
}

NLayout_Error_Info nlayout_compile_text_style(
  nlayout_runtime_t runtime,
  const nlayout_text_style_t text_style,
  skia::textlayout::TextStyle *style)
{
  const uint8_t font_id = text_style.font_id;
  if (font_id >= runtime->fonts.count)
  {
    return nlayout_err_invalid_instruction;
  }
  const auto font_size = runtime->fonts.fonts[font_id].font_size;
  const auto font_style = runtime->fonts.fonts[font_id].font_style;

  style->setFontSize(font_size);
  style->setFontFamilies({ SkString(runtime->fonts.fonts[font_id].name) });
  style->addFontFeature(SkString("liga"), 1);
  style->addFontFeature(SkString("clig"), 1);
  style->addFontFeature(SkString("dlig"), 1);
  style->addFontFeature(SkString("hlig"), 1);
  style->addFontFeature(SkString("calt"), 1);

  SkFontStyle fs;
  if (font_style & nlayout_font_bold && font_style & nlayout_font_italic)
  {
    fs = SkFontStyle::BoldItalic();
  }
  else if (font_style & nlayout_font_italic)
  {
    fs = SkFontStyle::Italic();
  }
  else if (font_style & nlayout_font_bold)
  {
    fs = SkFontStyle::Bold();
  }
  else
  {
    fs = SkFontStyle::Normal();
  }
  style->setFontStyle(fs);

  skia::textlayout::TextDecoration decoration;
  if (font_style & nlayout_font_underline && font_style & nlayout_font_strike_through)
  {
    decoration =
      static_cast<skia::textlayout::TextDecoration>(
        skia::textlayout::TextDecoration::kUnderline |
        skia::textlayout::TextDecoration::kLineThrough);
  }
  else if (font_style & nlayout_font_underline)
  {
    decoration = skia::textlayout::TextDecoration::kUnderline;
  }
  else if (font_style & nlayout_font_strike_through)
  {
    decoration = skia::textlayout::TextDecoration::kLineThrough;
  }
  else
  {
    decoration = skia::textlayout::TextDecoration::kNoDecoration;
  }
  style->setDecoration(decoration);

  SkPaint bg, fg;
  const nlayout_color_t *bg_color = &text_style.bg_color;
  const nlayout_color_t *fg_color = &text_style.fg_color;
  bg.setColor(SkColorSetARGB(bg_color->alpha, bg_color->red, bg_color->green, bg_color->blue));
  fg.setColor(SkColorSetARGB(fg_color->alpha, fg_color->red, fg_color->green, fg_color->blue));
  style->setBackgroundColor(bg);
  style->setForegroundColor(fg);

  return nlayout_okay;
}

nlayout_text_style_t nlayout_apply_override(const nlayout_text_style_t style,
  const nlayout_style_override_map_t *map)
{
  if (map == nullptr)
  {
    return style;
  }
  for (uint8_t i = 0; i < map->count; ++i)
  {
    if (memcmp(&style, &map->list[i].original, sizeof(nlayout_text_style_t)) == 0)
    {
      return map->list[i].override;
    }
  }
  return style;
}

NLayout_Error_Info nlayout_extract_text_style(
  nlayout_runtime_t runtime,
  const nlayout_ins_asset_u *ins,
  const nlayout_style_override_map_t *map,
  skia::textlayout::TextStyle *style)
{
  return nlayout_compile_text_style(runtime,
    nlayout_apply_override(ins->text_style, map), style);
}

NLayout_Error_Info nlayout_extract_overrides(
  nlayout_runtime_t runtime,
  const nlayout_instruction_t *ins,
  nlayout_style_override_map_t *h, nlayout_style_override_map_t *b)
{
  (void)runtime;
  *h = ins->asset.con_style.s_override_heading;
  *b = ins->asset.con_style.s_override_body;
  return nlayout_okay;
}

NLayout_Error_Info nlayout_extract_con_geometry(
  nlayout_runtime_t runtime,
  const nlayout_instruction_t *ins,
  float *dx0, float *dx1, float *dy0, float *dy1)
{
  (void)runtime;
  *dx0 = ins->asset.con_style.padding_left;
  *dx1 = ins->asset.con_style.padding_right;
  *dy0 = ins->asset.con_style.padding_top;
  *dy1 = ins->asset.con_style.padding_bottom;
  return nlayout_okay;
}
