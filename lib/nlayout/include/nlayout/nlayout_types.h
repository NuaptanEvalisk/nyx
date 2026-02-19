//
// Created by felix on 2/14/26.
//

#ifndef NYX_NLAYOUT_TYPES_H
#define NYX_NLAYOUT_TYPES_H

#include <stddef.h> // NOLINT(*-deprecated-headers)
#include <stdint.h> // NOLINT(*-deprecated-headers)

#if !defined(__cplusplus)
#include <stdbool.h>
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

  typedef struct
  {
    int width;
    int height;
    float padding_left;
    float padding_right;
    bool scan_os_fonts;
    bool wrap_pixels;
  } nlayout_global_config_t;

  enum NLayout_Font_Style
  {
    nlayout_font_none = 0,
    nlayout_font_bold = 1 << 0,
    nlayout_font_italic = 1 << 1,
    nlayout_font_underline = 1 << 2,
    nlayout_font_strike_through = 1 << 3
  };

  typedef struct
  {
    char *name;
    size_t name_len;
    uint8_t font_style;
    float font_size;

    char **font_files;
    uint8_t font_files_count;
  } nlayout_font_entry_t;

  typedef struct
  {
    nlayout_font_entry_t *fonts;
    uint8_t count;
  } nlayout_font_registry_t;

  typedef size_t str_length_t;

  enum NLayout_Asset_Type
  {
    nlayout_asset_none,
    nlayout_asset_text_style,
    nlayout_asset_inline_box,
    nlayout_asset_display_box,
    nlayout_asset_container_op,
    nlayout_asset_container_style,
    nlayout_asset_cursor
  };

  typedef struct
  {
    float width;
    float height;
    float baseline_shift;
    bool resizable;
  } nlayout_box_t;

  typedef struct
  {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
  } nlayout_color_t;

  typedef struct
  {
    uint8_t font_id;
    nlayout_color_t fg_color;
    nlayout_color_t bg_color;
  } nlayout_text_style_t;

  typedef struct
  {
    nlayout_text_style_t original;
    nlayout_text_style_t override;
  } nlayout_style_override_entry_t;

  typedef struct
  {
    nlayout_style_override_entry_t *list;
    uint8_t count;
  } nlayout_style_override_map_t;

  typedef struct
  {
    nlayout_color_t bg_color;
    nlayout_color_t border_color;
    float border_thickness;
    float padding_top;
    float padding_bottom;
    float padding_left;
    float padding_right;
    nlayout_style_override_map_t s_override_heading;
    nlayout_style_override_map_t s_override_body;
  } nlayout_con_style_t;

  enum NLayout_Container_Ops
  {
    nlayout_container_begin,
    nlayout_container_end,
    nlayout_container_head,
    nlayout_container_body
  };

  typedef union
  {
    nlayout_text_style_t text_style;
    nlayout_con_style_t con_style;
    nlayout_box_t box;
    enum NLayout_Container_Ops con_operation;
  } nlayout_ins_asset_u;

  typedef struct
  {
    str_length_t offset;
    enum NLayout_Asset_Type asset_type;
    nlayout_ins_asset_u asset;
  } nlayout_instruction_t;

  typedef struct
  {
    float x;
    float y;
    float width;
  } nlayout_box_data_t;

  typedef struct
  {
    float x;
    float y;
    float height;
  } nlayout_cursor_t;

  typedef struct
  {
    uint32_t *pixels; // RGBA8888
    int width;
    int height;
    int stride;
  } nlayout_buffer_t;

  typedef struct nlayout_hit_t
  {
    str_length_t main_offset;
    str_length_t ins_offset;
  } nlayout_hit_t;

  // opaque pointer
  typedef struct nlayout_runtime_internal *nlayout_runtime_t;

#if defined(__cplusplus)
}
#endif

#endif // NYX_NLAYOUT_TYPES_H