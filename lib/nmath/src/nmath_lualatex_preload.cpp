//
// Created by felix on 2/19/26.
//

#include "nmath_internals.hpp"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

std::string generate_ini_command(nmath_runtime_t runtime)
{
  const fs::path dump_path(runtime->globals.lualatex_dump_path);
  const std::string output_dir = dump_path.parent_path().string();
  const std::string job_name = dump_path.stem().string();
  const std::string source_tex = runtime->globals.tex_fmt_source_path;

  std::string cmd = "" +std::string(runtime->globals.lualatex_exe_path) +
                    " -ini -interaction=nonstopmode -halt-on-error "
                    "-jobname=" + job_name + " "
                    "-output-directory=" + output_dir + " "
                    "\"&pdflatex\" " + source_tex + " > /dev/null 2>&1";
  return cmd;
}

enum NMath_Error_Info nmath_lualatex_precompile(nmath_runtime_t runtime)
{
  if (runtime == nullptr || runtime->globals.lualatex_exe_path == nullptr ||
    runtime->globals.lualatex_dump_path == nullptr ||
    runtime->globals.tex_fmt_source_path == nullptr ||
    runtime->globals.tex_head_path == nullptr ||
    runtime->globals.tex_tail_path == nullptr)
  {
    return nmath_err_null_check;
  }

  const std::string cmd = generate_ini_command(runtime);
  if (const int ret = std::system(cmd.c_str()); ret != 0)
  {
    return nmath_err_lualatex_fmt_failure;
  }

  if (!fs::exists(runtime->globals.lualatex_dump_path))
  {
    return nmath_err_lualatex_fmt_missing;
  }

  return nmath_okay;
}
