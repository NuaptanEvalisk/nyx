//
// Created by felix on 2/19/26.
//

#include "nmath_internals.hpp"
#include <fstream>
#include <regex>
#include <string>

#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-page-renderer.h>

#include <ctime>

static NMath_Error_Info nmath_render_pdf_to_bitmap(
    const char* pdf_path,
    float scale_factor,
    nmath_bitmap_t* out_bitmap,
    nmath_runtime_t runtime)
{
  const auto doc = poppler::document::load_from_file(pdf_path);
  if (!doc)
  {
    return nmath_err_invalid_pdf;
  }

  const poppler::page *page = doc->create_page(0);
  if (!page)
  {
    return nmath_err_invalid_pdf;
  }

  const double dpi = 72.0 * scale_factor;

  poppler::image img = runtime->page_renderer.render_page(page, dpi, dpi);

  if (!img.is_valid())
  {
    return nmath_err_invalid_pdf;
  }

  const int w = img.width();
  const int h = img.height();
  const int stride = img.bytes_per_row();
  const size_t size = stride * h;

  out_bitmap->pixels = static_cast<uint32_t *>(malloc(size));
  if (!out_bitmap->pixels)
  {
    return nmath_err_failed_alloc;
  }

  memcpy(out_bitmap->pixels, img.data(), size);
  out_bitmap->width = w;
  out_bitmap->height = h;
  out_bitmap->stride = stride;
  out_bitmap->data_size = size;

  return nmath_okay;
}

inline double get_ms(const struct timespec start, const struct timespec end)
{
  return static_cast<double>(end.tv_sec - start.tv_sec) * 1000.0 +
         static_cast<double>(end.tv_nsec - start.tv_nsec) / 1000000.0;
}

enum NMath_Error_Info nmath_render_quality(const char *formula,
                                           const enum NMath_Formula_Type ft,
                                           nmath_bitmap_t *bitmap,
                                           nmath_runtime_t runtime)
{
  struct timespec t0, t1;

  if (formula == nullptr || runtime == nullptr)
  {
    return nmath_err_null_check;
  }

  clock_gettime(CLOCK_MONOTONIC, &t0);
  {
    std::ofstream ofs(runtime->tex_path, std::ios::trunc);
    if (!ofs.is_open())
    {
      return nmath_err_file;
    }

    ofs << runtime->latex_head_content;
    ofs << (ft == nmath_formula_inline ? "$" : "$\\displaystyle ");
    ofs << formula;
    ofs << "$";
    ofs << runtime->latex_tail_content;
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  std::cout << "Construction of mwe: " << get_ms(t0, t1) << " ms" << std::endl;

  // routing
  const std::string f(formula);
  bool heavy = f.find("tikz") != std::string::npos ||
               f.find("draw") != std::string::npos ||
               f.find("includegraphics") != std::string::npos ||
               f.find("color") != std::string::npos ||
               f.find("xlongequal") != std::string::npos ||
               f.find("degree") != std::string::npos ||
               f.find("celsius") != std::string::npos ||
               f.find("ohm") != std::string::npos ||
               f.find("perthousand") != std::string::npos ||
               f.find("micro") != std::string::npos;

  std::cout << "Routing: " << (heavy ? "heavy" : "mini") << std::endl;

  clock_gettime(CLOCK_MONOTONIC, &t0);
  if (!nmath_spawn_pdflatex(heavy ? runtime->latex_cmd_args : runtime->latex_cmd_mini_args))
  {
    return nmath_err_lualatex_failure;
  }
  clock_gettime(CLOCK_MONOTONIC, &t1);
  std::cout << "Exec of pdflatex: " << get_ms(t0, t1) << " ms" << std::endl;

  clock_gettime(CLOCK_MONOTONIC, &t0);
  const NMath_Error_Info e = nmath_render_pdf_to_bitmap(
      runtime->out_pdf_path.c_str(), runtime->globals.pdf_scale_factor, bitmap,
      runtime);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  std::cout << "Poppler reading pixels: " << get_ms(t0, t1) << " ms" << std::endl;

  return e;
}