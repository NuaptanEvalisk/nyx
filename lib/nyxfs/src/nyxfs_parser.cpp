//
// Created by felix on 2/23/26.
//

#include "nyxfs_internals.hpp"
#include <ranges>
#include <vector>
#include <cstring>
#include <cassert>

static inline __attribute__((always_inline))
char *next_line_starter(char *i)
{
  for (; *i != '\0'; ++i)
  {
    if (*i == '\n')
    {
      return i + 1;
    }
  }
  return nullptr;
}

static inline __attribute__((always_inline))
void dequote1(char *text)
{
  for (; text != nullptr && *text != '\0'; text = next_line_starter(text))
  {
    if (text[0] != '>')
    {
      continue;
    }
    for (size_t i = 0; text[i] != '\0'; ++i)
    {
      text[i] = text[i + 1];
    }
  }
}

NyxFS_Error_Info nyxfs_enstructure(nyxfs_arena_t *arena, nyxfs_node_t *node)
{
  // we make containers. to do so we highlight lines beginning with `>` that is NOT in a code block
  // AND NOT in a TeX formula.
  char *text = node->data.raw_data.text_offset;
  std::vector<std::pair<uint8_t, char *>> master;
  bool in_tex = false, in_code = false;
  for (char *t = text; t != nullptr && *t != '\0'; t = next_line_starter(t))
  {
    if (t[0] == '$' && t[1] == '$')
    {
      in_tex = !in_tex;
    }
    else if (t[0] == '`' && t[1] == '`' && t[2] == '`')
    {
      in_code = !in_code;
    }
    master.emplace_back(!in_tex && !in_code && t[0] == '>', t);
  }

  if (master.empty())
  {
    return nyxfs_okay;
  }

  uint8_t cur = master[0].first;
  for (size_t k = 0; k < master.size(); ++k)
  {
    if (auto& [i, cx] = master[k]; i != cur || k == master.size() - 1)
    {
      cur = i;
      cx[-1] = '\0';  // cx is beginning of new line, so cx - 1 is the last \n.
      nyxfs_node_t *rnode;
      if (i)
      {
        nyxfs_node_t *cnode = nyxfs_node_create_child(node, arena);
        if (cnode == nullptr)
        {
          return nyxfs_err_node_creation_failure;
        }
        cnode->type = nyxfs_node_container;
        rnode = nyxfs_node_create_first_child(cnode, arena);
        dequote1(cx);
        if (k == 0)
        {
          node->first_child = cnode;
        }
      }
      else
      {
        rnode = nyxfs_node_create_child(node, arena);
        if (k == 0)
        {
          node->first_child = rnode;
        }
      }
      if (rnode == nullptr)
      {
        return nyxfs_err_node_creation_failure;
      }
      rnode->type = nyxfs_node_raw;
      rnode->data.raw_data.text_offset = cx;
      nyxfs_enstructure(arena, rnode);
    }
  }
  return nyxfs_okay;
}

static inline __attribute__((always_inline))
int test_numeric_list(const char* s) {
  if (!s || !std::isdigit(*s))
  {
    return -1;
  }

  char* end_ptr = nullptr;
  const long val = std::strtol(s, &end_ptr, 10);

  if (end_ptr && end_ptr[0] == '.' && end_ptr[1] == ' ')
  {
    if (end_ptr[2] != '\0')
    {
      return static_cast<int>(val);
    }
  }
  return -1;
}

// assumes already enstructured.
NyxFS_Error_Info nyxfs_enlist(nyxfs_arena_t *arena, nyxfs_node_t *node)
{
  char *text = node->data.raw_data.text_offset;
  bool in_tex = false, in_code = false;
  char *t = text;
  for (; t != nullptr && *t != '\0'; t = next_line_starter(t))
  {
    if (t[0] == '$' && t[1] == '$')
    {
      in_tex = !in_tex;
    }
    else if (t[0] == '`' && t[1] == '`' && t[2] == '`')
    {
      in_code = !in_code;
    }
    if (in_tex || in_code)
    {
      continue;
    }
    if (t[0] == '-' || test_numeric_list(t) != -1)
    {
      break;
    }
  }
  if (t == nullptr)
  {
    return nyxfs_okay;
  }
  nyxfs_node_t *par = node->parent;
  nyxfs_node_t *lst = nyxfs_node_create_first_child(par, arena);
  if (lst == nullptr)
  {
    return nyxfs_err_node_creation_failure;
  }
  lst->type = nyxfs_node_list;
  lst->data.list_data.id = t[0] == '-' ? -1 : test_numeric_list(t);
  if (par->type == nyxfs_node_list)
  {
    const auto pd = par->data.list_data.depth;
    const auto pid = par->data.list_data.id;
    lst->data.list_data.depth = pid >= lst->data.list_data.id ? pd + 1 : pd;
  }
  else
  {
    lst->data.list_data.depth = 0;
  }

  // the logics above are incorrect: an actual listing looks like this:
  // ------------------------------------------------------------------
  // 1. something
  //   1. something more
  //     - even more
  //
  // irrelevant text
  // ------------------------------------------------------------------
  return nyxfs_err_unimplemented;
}

static inline __attribute__((always_inline))
bool starts_with(const char *pre, const char *str)
{
  const size_t len_pre = strlen(pre);
  const size_t len_str = strlen(str);
  if (len_pre > len_str)
  {
    return false;
  }
  return strncmp(pre, str, len_pre) == 0;
}

static inline __attribute__((always_inline))
bool detect_and_flip(const char *detect, const char *text, bool *flip,
  nyxfs_token_t **tok_active, const NyxFS_Token_Type tt, nyxfs_arena_t *arena)
{
  if (!starts_with(detect, text))
  {
    return false;
  }
  *flip = !*flip;
  if (*flip)
  {
    *tok_active = static_cast<nyxfs_token_t*>(nyxfs_arena_alloc(arena, sizeof(nyxfs_token_t)));
    if (*tok_active == nullptr)
    {
      return false;
    }
    (*tok_active)->type = tt;
    (*tok_active)->data_length = 0;
  }
  else
  {
    nyxfs_arena_pad8(arena);
  }
  return true;
}

// this corresponds to the usual "inline parsing" thing. node is a stream node
// whose tok_begin indicates the text to parse.
/*
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
*/
NyxFS_Error_Info nyxfs_tokenize(nyxfs_arena_t *arena, nyxfs_node_t *node)
{
  bool reg_bold = false, reg_italic = false, reg_underline = false, reg_sout = false;
  bool reg_highlight = false, reg_itex = false, reg_dtex = false, reg_icode = false;
  bool reg_dcode = false;
  const char *text = node->data.raw_data.text_offset;
  nyxfs_token_t *tok_active = nullptr;
  for (size_t i = 0; text[i] != '\0'; ++i)
  {
    if (detect_and_flip("$$", text + i, &reg_dtex, &tok_active, nyxfs_token_display_formula, arena) ||
      detect_and_flip("```", text + i, &reg_dcode, &tok_active, nyxfs_token_display_code, arena) ||
      detect_and_flip("$", text + i, &reg_itex, &tok_active, nyxfs_token_inline_formula, arena) ||
      detect_and_flip("`", text + i, &reg_icode, &tok_active, nyxfs_token_inline_code, arena))
    {
      continue;
    }

    if (reg_dtex || reg_dcode || reg_itex || reg_icode)
    {
      nyxfs_arena_append1(arena, text[i]);
      ++tok_active->data_length;
      continue;
    }


  }
}

// when enters this, already know the raw thing should be just paragraphs
// this function separates a raw node into a few stream nodes, and enters
// each as it processes. the paragraph rule is simple: 2+ consecutive line
// breaks indicate a paragraph shift.
// e.g.
// ------------------------------------------------------------------
// ttttttt\n\n\ntttttttttttt\n\n\nttttttttttttttttttttt
//           ^ (i) detects
//           \0 (ii) replaces this with \0
// ttttttt\n\0\ntttttttttttt\n\n\nttttttttttttttttttttt
// natstr1   | (iii) proc rest, enters if strlen > 0
// ------------------------------------------------------------------

NyxFS_Error_Info nyxfs_enstream(nyxfs_arena_t *arena, nyxfs_node_t *node)
{
  char *text = node->data.raw_data.text_offset;
  bool in_tex = false, in_code = false;
  nyxfs_node_t *first = nullptr;
  for (char *t = text; t != nullptr && *t != '\0';)
  {
    if (t[0] == '$' && t[1] == '$')
    {
      in_tex = !in_tex;
    }
    else if (t[0] == '`' && t[1] == '`' && t[2] == '`')
    {
      in_code = !in_code;
    }
    if (in_tex || in_code)
    {
      continue;
    }
    if(t[0] != '\n')
    {
      // normal line break;
      t = next_line_starter(t);
      continue;
    }
    t[0] = '\0';
    if (strlen(text) == 0)
    {
      for (text = t + 1; *text == '\n'; ++text) {}
      t = text;
      continue;
    }
    nyxfs_node_t *s = nyxfs_node_create_sibling(node, arena);
    if (s == nullptr)
    {
      return nyxfs_err_node_creation_failure;
    }
    if (first == nullptr)
    {
      first = s;
    }
    s->type = nyxfs_node_stream;
    s->data.stream_data.tok_begin = text;
    if (NyxFS_Error_Info e = nyxfs_tokenize(arena, s); e != nyxfs_okay)
    {
      return e;
    }
    for (text = t + 1; *text == '\n'; ++text) {}
    t = text;
  }
  node->parent->first_child = first;
  return nyxfs_okay;
}