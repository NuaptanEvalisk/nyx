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

static bool extract_geometry_from_log(const std::string &log,
                                      nmath_geometry_t *geo)
{
  std::ifstream log_file(log);
  if (!log_file.is_open())
  {
    return false;
  }

  std::string line;
  std::regex re(
      "NGEO:\\s*w=([0-9.]+)\\s*pt,\\s*h=([0-9.]+)\\s*pt,\\s*d=([0-9.]+)\\s*pt");
  std::smatch match;

  while (std::getline(log_file, line))  {
    if (std::regex_search(line, match, re))
    {
      geo->width = std::stof(match[1]);
      geo->height = std::stof(match[2]);
      geo->baseline = std::stof(match[3]);
      return true;
    }
  }
  return false;
}

static NMath_Error_Info nmath_render_pdf_to_bitmap(
    const char* pdf_path,
    float scale_factor,
    nmath_bitmap_t* out_bitmap)
{
  const auto doc = poppler::document::load_from_file(pdf_path);
  if (!doc)
  {
    return nmath_err_invalid_pdf;
  }

  poppler::page *page = doc->create_page(0);
  if (!page)
  {
    return nmath_err_invalid_pdf;
  }

  const double dpi = 72.0 * scale_factor;

  poppler::page_renderer renderer;
  renderer.set_render_hint(poppler::page_renderer::text_antialiasing);
  renderer.set_image_format(poppler::image::format_argb32);
  poppler::image img = renderer.render_page(page, dpi, dpi);

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

enum NMath_Error_Info nmath_render_quality(const char *formula,
                                           const enum NMath_Formula_Type ft,
                                           nmath_bitmap_t *bitmap,
                                           nmath_geometry_t *geometry,
                                           nmath_runtime_t runtime)
{
  if (formula == nullptr || geometry == nullptr || runtime == nullptr)
  {
    return nmath_err_null_check;
  }

  std::string tex_path, out_pdf_path, log_path;

  tex_path += runtime->globals.lualatex_shm_dir;
  tex_path += "/";
  tex_path += runtime->globals.lualatex_job_name;
  tex_path += ".tex";

  out_pdf_path += runtime->globals.lualatex_shm_dir;
  out_pdf_path += "/";
  out_pdf_path += runtime->globals.lualatex_job_name;
  out_pdf_path += ".pdf";

  log_path += runtime->globals.lualatex_shm_dir;
  log_path += "/";
  log_path += runtime->globals.lualatex_job_name;
  log_path += ".log";

  {
    std::ofstream ofs(tex_path, std::ios::trunc);
    if (!ofs.is_open())
    {
      return nmath_err_file;
    }

    ofs << runtime->latex_head_content;
    ofs << (ft == nmath_formula_inline ? "$" : "\n$$\n");
    ofs << formula;
    ofs << (ft == nmath_formula_inline ? "$" : "\n$$\n");
    ofs << runtime->latex_tail_content;
  }

  std::string cmd = "lualatex -interaction=batchmode -halt-on-error "
                    "-fmt=" + std::string(runtime->globals.lualatex_dump_path) + " "
                    "-output-directory=" + runtime->globals.lualatex_shm_dir + " "
                    + tex_path + " > /dev/null 2>&1";

  if (const int ret = std::system(cmd.c_str()); ret != 0)
  {
    return nmath_err_lualatex_failure;
  }
  if (!extract_geometry_from_log(log_path, geometry))
  {
    return nmath_err_geom_extraction;
  }

  const NMath_Error_Info e = nmath_render_pdf_to_bitmap(
      out_pdf_path.c_str(), runtime->globals.pdf_scale_factor, bitmap);
  return e;
}