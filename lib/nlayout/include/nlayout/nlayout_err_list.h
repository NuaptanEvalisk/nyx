//
// Created by felix on 2/16/26.
//

#ifndef NYX_NLAYOUT_ERR_LIST_H
#define NYX_NLAYOUT_ERR_LIST_H

#define NLAYOUT_ERROR_LIST(X) \
X(nlayout_okay) \
X(nlayout_err_null_check) \
X(nlayout_err_failed_alloc) \
X(nlayout_err_invalid_dimensions) \
X(nlayout_err_invalid_font) \
X(nlayout_err_invalid_icu_data) \
X(nlayout_err_invalid_instruction) \
X(nlayout_err_no_available_box) \
X(nlayout_err_invalid_box_content) \
X(nlayout_err_buffer_too_small) \
X(nlayout_err_pixel_reading_failure) \
X(nlayout_err_invalid_lanes) \
X(nlayout_err_hit_out_of_range_up) \
X(nlayout_err_hit_out_of_range_down) \
X(nlayout_err_impossible) \
X(nlayout_err_failed_to_wrap_pixels)

#define GENERATE_ENUM(ENUM) ENUM,
enum NLayout_Error_Info
{
  NLAYOUT_ERROR_LIST(GENERATE_ENUM)
};
#undef GENERATE_ENUM

#define GENERATE_STRING(STRING) case STRING: return #STRING;

// if no static, would cause C linker error
static inline const char* nlayout_explain_err(const enum NLayout_Error_Info err)
{
  switch (err)
  {
    NLAYOUT_ERROR_LIST(GENERATE_STRING)
    default: return "nlayout_err_unknown";
  }
}

#undef GENERATE_STRING

#endif // NYX_NLAYOUT_ERR_LIST_H
