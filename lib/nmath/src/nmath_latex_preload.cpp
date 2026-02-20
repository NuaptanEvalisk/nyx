//
// Created by felix on 2/19/26.
//

#include "nmath_internals.hpp"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

std::string generate_ini_command(nmath_runtime_t runtime)
{
  const fs::path dump_path(runtime->globals.latex_dump_path);
  const std::string output_dir = dump_path.parent_path().string();
  const std::string job_name = dump_path.stem().string();
  const std::string source_tex = runtime->globals.tex_fmt_source_path;

  std::string cmd = "" +std::string(runtime->globals.latex_exe_path) +
                    " -ini -interaction=nonstopmode -halt-on-error "
                    "-jobname=" + job_name + " "
                    "-output-directory=" + output_dir + " "
                    "\"&pdflatex\" " + source_tex + " > /dev/null 2>&1";
  return cmd;
}

std::string generate_ini_command_mini(nmath_runtime_t runtime)
{
  const fs::path dump_path(runtime->globals.latex_dump_mini_path);
  const std::string output_dir = dump_path.parent_path().string();
  const std::string job_name = dump_path.stem().string();
  const std::string source_tex = runtime->globals.tex_fmt_mini_path;

  std::string cmd = "" +std::string(runtime->globals.latex_exe_path) +
                    " -ini -interaction=nonstopmode -halt-on-error "
                    "-jobname=" + job_name + " "
                    "-output-directory=" + output_dir + " "
                    "\"&pdflatex\" " + source_tex + " > /dev/null 2>&1";
  return cmd;
}

static bool copy_minimal_map(nmath_runtime_t runtime)
{
  if (runtime->globals.latex_shm_dir == nullptr ||
    runtime->globals.tex_minimal_map_path == nullptr)
  {
    return false;
  }

  if (!std::filesystem::copy_file(
          std::string(runtime->globals.tex_minimal_map_path),
          std::string(runtime->globals.latex_shm_dir) + "/minimal.map",
          std::filesystem::copy_options::overwrite_existing))
  {
    return false;
  }
  debug_info_msg("Copied " << std::string(runtime->globals.tex_minimal_map_path)
    << " to " << std::string(runtime->globals.latex_shm_dir) << "/minimal.map");
  return true;
}

enum NMath_Error_Info nmath_lualatex_precompile(nmath_runtime_t runtime)
{
  if (runtime == nullptr || runtime->globals.latex_exe_path == nullptr ||
    runtime->globals.latex_dump_path == nullptr ||
    runtime->globals.tex_fmt_source_path == nullptr ||
    runtime->globals.tex_head_path == nullptr ||
    runtime->globals.tex_tail_path == nullptr)
  {
    return nmath_err_null_check;
  }

  if (!copy_minimal_map(runtime))
  {
    return nmath_err_minimal_map;
  }

  const std::string cmd = generate_ini_command(runtime);
  if (const int ret = std::system(cmd.c_str()); ret != 0)
  {
    return nmath_err_lualatex_fmt_failure;
  }

  const std::string cmd_mini = generate_ini_command_mini(runtime);
  if (const int ret = std::system(cmd_mini.c_str()); ret != 0)
  {
    return nmath_err_lualatex_fmt_failure;
  }

  if (!(fs::exists(runtime->globals.latex_dump_path) ||
    fs::is_directory(runtime->globals.latex_dump_mini_path)))
  {
    return nmath_err_lualatex_fmt_missing;
  }

  return nmath_okay;
}
