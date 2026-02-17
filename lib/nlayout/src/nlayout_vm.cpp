//
// Created by felix on 2/15/26.
//

// todo: build text style cache

#include "nlayout_internals.hpp"
#include <cassert>
#include <vector>
#include <cstring>

NLayout_Error_Info
nlayout_layout(nlayout_runtime_t runtime, const char *text,
               str_length_t text_len,
               // ReSharper disable once CppParameterMayBeConstPtrOrRef
               nlayout_instruction_t *ins, str_length_t ins_count)
{
  if (text == nullptr || ins == nullptr)
  {
    return nlayout_err_null_check;
  }

  skia::textlayout::ParagraphStyle para_style;
  para_style.setTextAlign(skia::textlayout::TextAlign::kJustify);

  bool has_cursor = false;
  str_length_t cursor_pos = 0;
  bool cursor_prepared = false;
  float cursor_x = 0, cursor_y = 0, cursor_h = 0;

  nlayout_style_override_map_t override_h_map;
  nlayout_style_override_map_t override_b_map;
  nlayout_style_override_map_t *effective_override_map = nullptr;

  float delta_x0 = 0, delta_x1 = 0;
  float delta_y0 = 0, delta_y1 = 0;

  str_length_t i = 0, j = 0;
  float x0 = runtime->globals.padding_left;
  float x1 = runtime->globals.padding_right;
  float y0 = 0;

  float x0_a = 0, x1_a = 0, y0_a = 0;
  bool padding_adjust = false;

  nlayout_color_t rect_bg_color, rect_border_color;
  float rect_border_thickness = 0;
  bool draw_con_rect = false;

  bool reg_dbox = false;
  float dbox_yskip = 0;
  float dbox_width = 0;

  skia::textlayout::TextStyle style;

  debug_info_msg("NLayout VM begins.");
  while (i < text_len)
  {
    const auto builder =
      skia::textlayout::ParagraphBuilder::make(
        para_style, runtime->font_collection, runtime->unicode);
    builder->pushStyle(style);

    const str_length_t p_base = i;

    for (; i < text_len; ++i)
    {
      if (padding_adjust)
      {
        x0 = x0_a, x1 = x1_a, y0 = y0_a;
        padding_adjust = false;
      }

      for (; j < ins_count; ++j)
      {
        if (ins[j].offset != i)
        {
          break;
        }

        switch (ins[j].asset_type)
        {
          case nlayout_asset_text_style:
          {
            debug_info_msg("offset = " << i << " instruction = " << j << " text_style.");
            if (nlayout_extract_text_style(runtime, &ins[j].asset,
              effective_override_map, &style) != nlayout_okay)
            {
              return nlayout_err_invalid_instruction;
            }
            builder->pop();
            builder->pushStyle(style);
          }
          break;

          case nlayout_asset_inline_box:
          {
            debug_info_msg("offset = " << i << " instruction = " << j << " ibox.");
            skia::textlayout::PlaceholderStyle placeholder;
            nlayout_extract_placeholder(runtime, &ins[j].asset, &placeholder);
            builder->addPlaceholder(placeholder);
            break;
          }

          case nlayout_asset_display_box:
          {
            debug_info_msg("offset = " << i << " instruction = " << j << " dbox.");
            reg_dbox = true;
            dbox_width = ins[j].asset.box.width;
            dbox_yskip = ins[j].asset.box.height;
            break;
          }

          case nlayout_asset_cursor:
          {
            debug_info_msg("offset = " << i << " instruction = " << j << " cursor.");
            has_cursor = true;
            cursor_pos = i - p_base;  // relative to paragraph base position.
            break;
          }

          case nlayout_asset_container_op:
          {
            switch (ins[j].asset.con_operation)
            {
              case nlayout_container_begin:
              {
                debug_info_msg("offset = " << i << " instruction = " << j << " c_begin.");
                y0_a = y0 + delta_y0;
                x0_a = x0 + delta_x0, x1_a = x1 + delta_x1;
                padding_adjust = true;
                break;
              }

              case nlayout_container_end:
              {
                debug_info_msg("offset = " << i << " instruction = " << j << " c_end.");
                effective_override_map = nullptr;
                y0_a = y0 + delta_y1;
                x0_a = x0 - delta_x0, x1_a = x1 - delta_x1;
                padding_adjust = true;
                draw_con_rect = true;
                break;
              }

              case nlayout_container_head:
              {
                debug_info_msg("offset = " << i << " instruction = " << j << " c_head.");
                effective_override_map = &override_h_map;
                break;
              }

              case nlayout_container_body:
              {
                debug_info_msg("offset = " << i << " instruction = " << j << " c_body.");
                effective_override_map = &override_b_map;
                break;
              }
            }
            break;
          }

          case nlayout_asset_container_style:
          {
            debug_info_msg("offset = " << i << " instruction = " << j << " c_style..");
            nlayout_extract_overrides(runtime, &ins[j],
              &override_h_map, &override_b_map);
            nlayout_extract_con_geometry(runtime, &ins[j],
              &delta_x0, &delta_x1, &delta_y0, &delta_y1);
            rect_bg_color = ins[j].asset.con_style.bg_color;
            rect_border_color = ins[j].asset.con_style.border_color;
            rect_border_thickness = ins[j].asset.con_style.border_thickness;
            break;
          }

          default:
            debug_info_msg("offset = " << i << " instruction = " << j << " unfdefned.");
            break;
        }

        if (padding_adjust || reg_dbox)
        {
          break;
        }
      }

      if (padding_adjust || reg_dbox)
      {
        ++j;
        break;
      }

      builder->addText(text + i, 1);
    }

    const auto paragraph = builder->Build();
    paragraph->layout(static_cast<float>(runtime->globals.width) - x0 - x1);

    if (draw_con_rect)
    {
      draw_con_rect = false;
      debug_info_msg("draw container rect at (" << x0 << "," << y0 << ").");

      SkRect con_rect = SkRect::MakeXYWH(x0, y0,
        static_cast<float>(runtime->globals.width) - x0 - x1,
        paragraph->getHeight());

      SkPaint paint_fill;
      paint_fill.setColor(SkColorSetARGB(rect_bg_color.alpha,
        rect_bg_color.red, rect_bg_color.green, rect_bg_color.blue));
      paint_fill.setAntiAlias(true);
      paint_fill.setStyle(SkPaint::kFill_Style);
      runtime->canvas->drawRect(con_rect, paint_fill);

      SkPaint paint_border;
      paint_border.setColor(SkColorSetARGB(rect_border_color.alpha,
        rect_border_color.red, rect_border_color.green,
        rect_border_color.blue));
      paint_border.setAntiAlias(true);
      paint_border.setStyle(SkPaint::kStroke_Style);
      paint_border.setStrokeWidth(rect_border_thickness);
      runtime->canvas->drawRect(con_rect, paint_border);
    }

    paragraph->paint(runtime->canvas, x0, y0);

    if (has_cursor && !cursor_prepared)
    {
      std::vector<skia::textlayout::TextBox> rects =
          paragraph->getRectsForRange(cursor_pos, cursor_pos + 1,
                                      skia::textlayout::RectHeightStyle::kMax,
                                      skia::textlayout::RectWidthStyle::kTight);
      cursor_prepared = true;
      assert(!rects.empty());
      cursor_x = rects[0].rect.fLeft + x0;
      cursor_y = rects[0].rect.fTop + y0;
      cursor_h = rects[0].rect.height();
    }

    auto ibox_rects = paragraph->getRectsForPlaceholders();
    for (auto & ibox_rect : ibox_rects)
    {
      runtime->box_data[runtime->box_data_count].x = x0 + ibox_rect.rect.fLeft;
      runtime->box_data[runtime->box_data_count].y = y0 + ibox_rect.rect.fTop;
      runtime->box_data[runtime->box_data_count].width = ibox_rect.rect.width();
      ++runtime->box_data_count;
    }

    // we extract lines from current paragraph, we simply put things in lanes,
    // and then use an extra step to compute the offsets.
    std::vector<skia::textlayout::LineMetrics> metrics;
    paragraph->getLineMetrics(metrics);

    for (const auto& line : metrics)
    {
      nlayout_ht_lane lane;
      lane.top = y0 + static_cast<float>(line.fBaseline - line.fAscent);
      lane.bottom = y0 + static_cast<float>(line.fBaseline + line.fDescent);
      lane.left = x0 + static_cast<float>(line.fLeft);
      lane.right = x0 + static_cast<float>(line.fLeft + line.fWidth);

      auto l_rects = paragraph->getRectsForRange(
        line.fStartIndex, line.fEndIndex,
        skia::textlayout::RectHeightStyle::kTight,
        skia::textlayout::RectWidthStyle::kTight);
      for (const auto& l_box : l_rects)
      {
        constexpr nlayout_hit_t pos0 = { 0, 0 };
        lane.inline_map.emplace_back( pos0, l_box.rect.left() );
      }

      runtime->ht_lanes.push_back(std::move(lane));
    }

    y0 += paragraph->getHeight();
    y0_a += paragraph->getHeight();

    if (reg_dbox)
    {
      reg_dbox = false;
      runtime->box_data[runtime->box_data_count].x = x0;
      runtime->box_data[runtime->box_data_count].y = y0;
      runtime->box_data[runtime->box_data_count].width = dbox_width;

      nlayout_ht_lane lane;
      lane.top = y0, lane.bottom = y0 + dbox_yskip;
      lane.left = x0, lane.right = x0 + dbox_width;
      runtime->ht_lanes.push_back(std::move(lane));

      y0 += dbox_yskip;
      ++runtime->box_data_count;
    }
  }

  if (has_cursor)
  {
    SkPaint paint;
    paint.setColor(SkColorSetRGB(0, 0, 0));
    paint.setStrokeWidth(2.0f);
    paint.setAntiAlias(true);
    runtime->canvas->drawLine(cursor_x, cursor_y,
      cursor_x, cursor_y + cursor_h, paint);
  }

  return nlayout_okay;
}

NLayout_Error_Info nlayout_iter_fill_box(nlayout_runtime_t runtime,
  const nlayout_buffer_t *content)
{
  if (runtime->box_data_count <= 0)
  {
    runtime->box_data_count = runtime->box_data_count_backup;
    runtime->box_data -= runtime->box_data_count;
    runtime->box_data_count_backup = 0;
    return nlayout_err_no_available_box;
  }

  const float bx = runtime->box_data[0].x;
  const float by = runtime->box_data[0].y;
  const float bw = runtime->box_data[0].width;

  const SkImageInfo info = SkImageInfo::Make(content->width, content->height,
                                           kRGBA_8888_SkColorType,
                                           kPremul_SkAlphaType);
  const SkPixmap pixmap(info, content->pixels, content->stride);
  const sk_sp<SkImage> image = SkImages::RasterFromPixmap(pixmap,
    nullptr, nullptr);
  if (image == nullptr)
  {
    return nlayout_err_invalid_box_content;
  }

  const float scale = bw / static_cast<float>(content->width);
  const float bh = static_cast<float>(content->height) * scale;
  const SkRect i_rect = SkRect::MakeXYWH(bx, by, bw, bh);
  SkPaint paint;
  runtime->canvas->drawImageRect(image, i_rect,
    SkSamplingOptions(SkFilterMode::kNearest), &paint);

  if (runtime->box_data_count_backup == 0)
  {
    runtime->box_data_count_backup = runtime->box_data_count;
  }

  ++runtime->box_data;
  --runtime->box_data_count;

  return nlayout_okay;
}

NLayout_Error_Info nlayout_export(nlayout_runtime_t runtime,
  const size_t allocated_buf_size, nlayout_buffer_t *out)
{
  if (out == nullptr || runtime->surface == nullptr)
  {
    return nlayout_err_null_check;
  }

  const int width = runtime->surface->width();
  const int height = runtime->surface->height();
  const int stride = width * 4;
  out->width = width, out->height = height, out->stride = stride;

  if (runtime->globals.wrap_pixels)
  {
    return nlayout_okay;
  }

  const SkImageInfo info = SkImageInfo::Make(width, height,
                                         kRGBA_8888_SkColorType,
                                         kPremul_SkAlphaType);

  if (allocated_buf_size < stride * height)
  {
    return nlayout_err_buffer_too_small;
  }
  memset(out->pixels, 0, allocated_buf_size);
  if (!runtime->surface->readPixels(info, out->pixels, stride, 0, 0))
  {
    return nlayout_err_pixel_reading_failure;
  }
  return nlayout_okay;
}