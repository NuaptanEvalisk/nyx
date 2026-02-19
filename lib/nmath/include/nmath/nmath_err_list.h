//
// Created by felix on 2/18/26.
//

#ifndef NYX_NMATH_ERR_LIST_H
#define NYX_NMATH_ERR_LIST_H

#define NMATH_ERROR_LIST(X) \
X(nmath_okay) \
X(nmath_err_null_check) \
X(nmath_err_file) \
X(nmath_err_failed_alloc) \
X(nmath_err_invalid_dimensions) \
X(nmath_err_wrap_pixels_failure) \
X(nmath_err_lualatex_fmt_failure) \
X(nmath_err_lualatex_fmt_missing) \
X(nmath_err_cannot_read_parts) \
X(nmath_err_lualatex_failure) \
X(nmath_err_geom_extraction) \
X(nmath_err_invalid_pdf)

#define GENERATE_ENUM(ENUM) ENUM,
enum NMath_Error_Info
{
  NMATH_ERROR_LIST(GENERATE_ENUM)
};
#undef GENERATE_ENUM

#define GENERATE_STRING(STRING) case STRING: return #STRING;

// if no static, would cause C linker error
static inline const char* nmath_explain_err(const enum NMath_Error_Info err)
{
  switch (err)
  {
    NMATH_ERROR_LIST(GENERATE_STRING)
    default: return "nmath_err_unknown";
  }
}

#undef GENERATE_STRING

#endif // NYX_NMATH_ERR_LIST_H
