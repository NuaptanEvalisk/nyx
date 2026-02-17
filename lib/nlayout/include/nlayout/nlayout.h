//
// Created by felix on 2/14/26.
//

#ifndef NYX_NLAYOUT_H
#define NYX_NLAYOUT_H

#include "nlayout_types.h"
#include "nlayout_err_list.h"

#if defined(__cplusplus)
extern "C"
{
#endif

  enum NLayout_Error_Info
  nlayout_runtime_create(const nlayout_global_config_t *globals,
                         nlayout_runtime_t *runtime, uint32_t *buffer);

  enum NLayout_Error_Info nlayout_runtime_reg_font(const nlayout_font_entry_t *font,
                                              nlayout_runtime_t runtime);

  enum NLayout_Error_Info
  nlayout_runtime_reset(const nlayout_global_config_t *globals,
                        nlayout_runtime_t runtime);

  enum NLayout_Error_Info nlayout_runtime_destroy(nlayout_runtime_t runtime);

  enum NLayout_Error_Info nlayout_layout(nlayout_runtime_t runtime, const char *text,
                                    str_length_t text_len,
                                    nlayout_instruction_t *ins,
                                    str_length_t ins_count);

  enum NLayout_Error_Info nlayout_compute_hits(nlayout_runtime_t runtime,
                                          str_length_t text_len,
                                          const nlayout_instruction_t *ins,
                                          str_length_t ins_count);

  enum NLayout_Error_Info nlayout_get_hit(nlayout_runtime_t runtime, float x,
                                     float y, nlayout_hit_t *hit);

  enum NLayout_Error_Info nlayout_iter_fill_box(nlayout_runtime_t runtime,
                                           const nlayout_buffer_t *content);

  enum NLayout_Error_Info nlayout_export(nlayout_runtime_t runtime,
                                    size_t allocated_buf_size,
                                    nlayout_buffer_t *out);

#if defined(__cplusplus)
}
#endif

#endif // NYX_NLAYOUT_H
