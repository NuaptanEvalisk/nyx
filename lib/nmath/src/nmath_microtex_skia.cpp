//
// Created by felix on 2/18/26.
//

// we implement the required graphics 2D API of MicroTeX so that it can use
// Skia backend.

#include <graphic/graphic.h>  // third_party/micro_tex/src/graphic/graphic.h
#include <include/core/SkCanvas.h>
#include <include/core/SkFont.h>
#include <include/core/SkFontMgr.h>
#include <include/core/SkRefCnt.h>
#include <include/core/SkPaint.h>
#include <include/core/SkTextBlob.h>
#include <include/core/SkTypeface.h>
#include "include/ports/SkFontMgr_empty.h"

#include <iostream>
#include <memory>

#include "nmath_internals.hpp"

thread_local SkFontMgr* g_tls_font_mgr = nullptr;

using namespace tex;

// ==========================================
// 1. Font mapping
// ==========================================
class SkiaFont : public tex::Font
{
public:
  SkFont skFont;

  SkiaFont(const std::string &file, float size)
  {
    const sk_sp<SkFontMgr> mgr = SkFontMgr_New_Custom_Empty();
    const sk_sp<SkTypeface> tf = mgr->makeFromFile(file.c_str());
    if (!tf)
    {
      std::cerr << "[nmath] Warning: Failed to load font file: " << file
                << std::endl;
    }
    skFont = SkFont(tf, size);
    skFont.setEdging(SkFont::Edging::kAntiAlias);
    skFont.setSubpixel(true);
  }

  SkiaFont(const std::string &name, int style, float size)
  {
    SkFontStyle skStyle;
    if (style == tex::BOLD)
    {
      skStyle = SkFontStyle::Bold();
    }
    else if (style == tex::ITALIC)
    {
      skStyle = SkFontStyle::Italic();
    }
    else if (style == tex::BOLDITALIC)
    {
      skStyle = SkFontStyle::BoldItalic();
    }
    else
    {
      skStyle = SkFontStyle::Normal();
    }

    sk_sp<SkTypeface> tf = nullptr;

    if (g_tls_font_mgr)
    {
      tf = g_tls_font_mgr->matchFamilyStyle(name.c_str(), skStyle);
    }

    if (!tf)
    {
      std::cerr << "[nmath] Warning: MicroTeX requested font '" << name
                << "', falling back to Empty." << std::endl;
      tf = SkTypeface::MakeEmpty();
    }

    skFont = SkFont(tf, size);
    skFont.setEdging(SkFont::Edging::kAntiAlias);
    skFont.setSubpixel(true);
  }

  [[nodiscard]] float getSize() const override
  {
    return skFont.getSize();
  }

  [[nodiscard]] sptr<tex::Font> deriveFont(int style) const override
  {
    return std::make_shared<SkiaFont>("", style, skFont.getSize());
  }

  bool operator==(const tex::Font &f) const override
  {
    auto sf = dynamic_cast<const SkiaFont *>(&f);
    return sf && skFont.getTypeface() == sf->skFont.getTypeface() &&
           skFont.getSize() == sf->skFont.getSize();
  }

  bool operator!=(const tex::Font &f) const override { return !(*this == f); }
};

tex::Font *tex::Font::create(const std::string &file, float size)
{
  return new SkiaFont(file, size);
}

sptr<tex::Font> tex::Font::_create(const std::string &name, int style,
                                   float size)
{
  return std::make_shared<SkiaFont>(name, style, size);
}

// ==========================================
// 2. TextLayout mapping
// ==========================================
class SkiaTextLayout : public tex::TextLayout
{
private:
  std::wstring text;
  const SkiaFont *font;
  SkRect bounds;

public:
  ~SkiaTextLayout() override = default;
  SkiaTextLayout(const std::wstring &src, const sptr<tex::Font> &f)
  {
    text = src;
    font = dynamic_cast<const SkiaFont *>(f.get());

    SkTextEncoding encoding = (sizeof(wchar_t) == 4) ? SkTextEncoding::kUTF32
                               // ReSharper disable once CppDFAUnreachableCode
                               : SkTextEncoding::kUTF16;
    font->skFont.measureText(text.c_str(), text.length() * sizeof(wchar_t),
                             encoding, &bounds);
  }

  void getBounds(tex::Rect &r) override
  {
    r.x = bounds.fLeft;
    r.y = bounds.fTop;
    r.w = bounds.width();
    r.h = bounds.height();
  }

  void draw(tex::Graphics2D &g2, float x, float y) override
  {
    g2.drawText(text, x, y);
  }
};

sptr<tex::TextLayout> tex::TextLayout::create(const std::wstring &src,
                                              const sptr<tex::Font> &font)
{
  return std::make_shared<SkiaTextLayout>(src, font);
}

// ==========================================
// 3. Graphics2D mapping
// ==========================================
class SkiaGraphics2D : public tex::Graphics2D
{
private:
  SkCanvas *canvas;
  SkPaint fillPaint;
  SkPaint strokePaint;
  const SkiaFont *currentFont = nullptr;
  tex::color currentColor = 0xFF000000; // black
  tex::Stroke currentStroke;
  SkM44 initialMatrix;

public:
  ~SkiaGraphics2D() override = default;
  SkiaGraphics2D(SkCanvas *c) : canvas(c)
  {
    fillPaint.setStyle(SkPaint::kFill_Style);
    fillPaint.setAntiAlias(true);

    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setAntiAlias(true);

    if (canvas)
    {
      initialMatrix = canvas->getLocalToDevice();
    }
  }

  void setColor(tex::color c) override
  {
    currentColor = c;
    fillPaint.setColor(c);
    strokePaint.setColor(c);
  }

  [[nodiscard]] tex::color getColor() const override
  {
    return currentColor;
  }

  void setStroke(const tex::Stroke &s) override
  {
    currentStroke = s;
    strokePaint.setStrokeWidth(s.lineWidth);

    switch (s.cap)
    {
    case tex::CAP_BUTT:
      strokePaint.setStrokeCap(SkPaint::kButt_Cap);
      break;
    case tex::CAP_ROUND:
      strokePaint.setStrokeCap(SkPaint::kRound_Cap);
      break;
    case tex::CAP_SQUARE:
      strokePaint.setStrokeCap(SkPaint::kSquare_Cap);
      break;
    }

    switch (s.join)
    {
    case tex::JOIN_BEVEL:
      strokePaint.setStrokeJoin(SkPaint::kBevel_Join);
      break;
    case tex::JOIN_ROUND:
      strokePaint.setStrokeJoin(SkPaint::kRound_Join);
      break;
    case tex::JOIN_MITER:
      strokePaint.setStrokeJoin(SkPaint::kMiter_Join);
      break;
    }
    strokePaint.setStrokeMiter(s.miterLimit);
  }

  [[nodiscard]] const tex::Stroke &getStroke() const override
  {
    return currentStroke;
  }

  void setStrokeWidth(float w) override
  {
    strokePaint.setStrokeWidth(w);
  }

  [[nodiscard]] const tex::Font *getFont() const override
  {
    return currentFont;
  }

  void setFont(const tex::Font *font) override
  {
    currentFont = dynamic_cast<const SkiaFont *>(font);
  }

  void translate(float dx, float dy) override
  {
    canvas->translate(dx, dy);
  }

  void scale(float sx, float sy) override
  {
    canvas->scale(sx, sy);
  }

  void rotate(float angle) override
  {
    canvas->rotate(angle * 180.0f / 3.1415926535f);
  }

  void reset() override
  {
    canvas->setMatrix(initialMatrix);
  }

  [[nodiscard]] float sx() const override
  {
    return canvas->getLocalToDevice().rc(0, 0);
  }

  [[nodiscard]] float sy() const override
  {
    return canvas->getLocalToDevice().rc(1, 1);
  }

  void drawText(const std::wstring &c, float x, float y) override
  {
    if (!currentFont || !canvas)
    {
      return;
    }
    SkFont &skFont = const_cast<SkiaFont *>(currentFont)->skFont;

    SkTextEncoding encoding = (sizeof(wchar_t) == 4) ? SkTextEncoding::kUTF32
                               // ReSharper disable once CppDFAUnreachableCode
                               : SkTextEncoding::kUTF16;
    canvas->drawSimpleText(c.c_str(), c.length() * sizeof(wchar_t), encoding, x,
                           y, skFont, fillPaint);
  }

  void drawLine(float x1, float y1, float x2, float y2) override
  {
    canvas->drawLine(x1, y1, x2, y2, strokePaint);
  }

  void drawRect(float x, float y, float w, float h) override
  {
    canvas->drawRect(SkRect::MakeXYWH(x, y, w, h), strokePaint);
  }

  void fillRect(float x, float y, float w, float h) override
  {
    canvas->drawRect(SkRect::MakeXYWH(x, y, w, h), fillPaint);
  }

  void drawRoundRect(float x, float y, float w, float h, float rx,
                     float ry) override
  {
    canvas->drawRoundRect(SkRect::MakeXYWH(x, y, w, h), rx, ry, strokePaint);
  }

  void fillRoundRect(float x, float y, float w, float h, float rx,
                     float ry) override
  {
    canvas->drawRoundRect(SkRect::MakeXYWH(x, y, w, h), rx, ry, fillPaint);
  }

  void rotate(float angle, float px, float py) override
  {
    const float degrees = angle * 180.0f / 3.14159265358979323846f;
    canvas->rotate(degrees, px, py);
  }

  void drawChar(wchar_t c, float x, float y) override
  {
    if (!currentFont || !canvas)
    {
      return;
    }

    const SkFont& skFont = const_cast<SkiaFont*>(currentFont)->skFont;
    constexpr auto encoding = SkTextEncoding::kUTF32;
    canvas->drawSimpleText(&c, sizeof(wchar_t), encoding, x, y, skFont, fillPaint);
  }
};

std::unique_ptr<Graphics2D> create_skia_graphics(SkCanvas* canvas)
{
  return std::make_unique<SkiaGraphics2D>(canvas);
}