//
// Created by felix on 2/21/26.
//

#ifndef NYX_NYXFS_TYPES_H
#define NYX_NYXFS_TYPES_H

#include <stdint.h> // NOLINT(*-deprecated-headers)

#if defined(__cplusplus)
extern "C"
{
#endif

  // NyxFS, aka. MegaAST abstracts directories and files as namespaces, which
  // are nodes in the AST as in nodes for lists and containers. nodes are loaded
  // and possibly, expanded (parsed), in a lazy, or even heuristic, manner.
  // we aim to design the memory layout in a compact way that with hardware in
  // mind and user in mind, the user being `nlayout`, our typesetting VM.
  // observe that in Obsidian-flavored Markdown, the structure is rather flat,
  // factually it should be little token streams attached to an AST tree
  // expressing the ordered-set structures.

  enum NyxFS_Node_Type
  {
    nyxfs_node_namespace,
    nyxfs_node_raw,
    nyxfs_node_lazy,
    nyxfs_node_stream,
    nyxfs_node_container,
    nyxfs_node_list,
    nyxfs_node_table,
    nyxfs_node_table_cell,
  };

  // we will maintain a global symbol table for namespace titles.
  // a directory is a namespace, a physical file is a namespace.
  struct nyxfs_namespace_data
  {
    char *symb_offset;
    uint16_t length;
  };

  // we will maintain a global arena for raw data.
  struct nyxfs_raw_data
  {
    char *text_offset;
    uint32_t length;
  };

  // to support lazy loading/parsing.
  struct nyxfs_lazy_data
  {
    uint16_t hint;
  };

  // a list node represents a list item, not full logical list. the list
  // hierarchy is encoded in the following `depth` field, not the arrangement
  // in MegaAST. `id` here means "3" in "3.", etc. if id < 0, this means the
  // item is unordered.
  // the "top" list item in MegaAST has the sublist of it being its first
  // children. by a sublist we mean either its logical-next, e.g. "4." after
  // "3.", or its logical-child, e.g. "4.1." (TAB 1.) after "4." item. this is
  // basically just recursive representation of a list. for a (sub)list, its
  // sibling does not belong to this (sub)list.
  struct nyxfs_list_data
  {
    uint8_t depth;
    int id;
  };

  // a table node has children being nodes with type table_cell. a table_cell
  // node has no data for it in nyxfs_node_data_u. it is a mere envelope of
  // the cell count.
  struct nyxfs_table_data
  {
    uint8_t columns_count;
  };

  // in construction of a node, our allocator would put the tokens right after
  // this struct. we design the memory of a stream in this way:
  // [stream_data] + [token] + [token] + [optional token asset] + [token], etc.
  struct nyxfs_stream_data
  {
    char *tok_begin;
    uint16_t tok_count;
  };

  // each container has two sections of children: header and body.
  // a callout is a container. a blockquote is a container.
  struct nyxfs_container_data
  {
    uint16_t header_children_count;
  };

  // why we use a union here instead of using purely manual allocation as in
  // token stream (below): nodes are rather sparse objects and a node is fat.
  // we do not have many nodes under a namespace, or under root node, comparing
  // with the tokens. unions are generally easier to manage.
  union nyxfs_node_data_u
  {
    nyxfs_namespace_data namespace_data;
    nyxfs_raw_data raw_data;
    nyxfs_lazy_data lazy_data;
    nyxfs_list_data list_data;
    nyxfs_table_data table_data;
    nyxfs_stream_data stream_data;
    nyxfs_container_data container_data;
  };

  typedef struct nyxfs_node
  {
    nyxfs_node *parent;
    nyxfs_node *first_child;
    nyxfs_node *next_sibling;

    NyxFS_Node_Type type;
    nyxfs_node_data_u data;
  } nyxfs_node_t;

  // fat node this is.
  static_assert(sizeof(nyxfs_node_t) == 48, "nyxfs_node_t should occupy 48 bytes.");

  // vitually, this is endowing tokens with parameters.
  // extlink: the \0 indicates separation between link text and address
  // image: the \0 indicates separation between location and size
  // wikilink: the \0 indicates separation between namespace, identifier,
  //           and then link text.
  // heading: the data would be a `uint8_t` indicating the level.
  enum NyxFS_Token_Type
  {
    nyxfs_token_text, nyxfs_token_bold_on, nyxfs_token_bold_off,
    nyxfs_token_italic_on, nyxfs_token_italic_off,
    nyxfs_token_underline_on, nyxfs_token_underline_off,
    nyxfs_token_strikeout_on, nyxfs_token_strikeout_off,
    nyxfs_token_highlight_on, nyxfs_token_highlight_off,
    nyxfs_token_newline, nyxfs_token_new_paragraph,
    nyxfs_token_extlink, nyxfs_token_wikilink, nyxfs_token_transclusion,
    nyxfs_token_image, nyxfs_token_heading, nyxfs_token_comment,
    nyxfs_token_inline_formula, nyxfs_token_display_formula,
    nyxfs_token_inline_code, nyxfs_token_display_code
  };

  // nyxfs_token *next can be calculated by data_begin and data_length.
  // we must do manual memory alignment here: our allocator will pad the data
  // by 8 bytes, and when calculating pointer `next`, this must also be taken
  // into account. in allocator's source we will create a macro like this:
  // #define NYXFS_ALIGN8(size) (((size) + 7) & ~7)
  // parsing is done when a file is active or gets preloaded, and is done
  // incrementally as we type, hence the memcpy of these text strings are fairly
  // acceptable, as pragmatic surveys have shown each text datium tends to be
  // quite short in a token stream. rendering happens every time, e.g. typing
  // (we use justified alignment), scrolling, etc. so if we merely store an
  // 8-byte pointer to `raw` arena here, pointer chasing would cause terrible
  // performance. hence this rather compact memory layout should be CPU-friendly
  typedef struct nyxfs_token
  {
    void *data_begin;
    NyxFS_Token_Type type;
    uint32_t data_length;
  } nyxfs_token_t;

  // roughly, 4 tokens nicely cached by CPU at once.
  static_assert(sizeof(nyxfs_token_t) == 16, "nyxfs_token should occupy 16 bytes");

#if defined(__cplusplus)
}
#endif

#endif // NYX_NYXFS_TYPES_H
