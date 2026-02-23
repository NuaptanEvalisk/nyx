//
// Created by felix on 2/23/26.
//

#include "nyxfs_internals.hpp"
#include <ranges>
#include <vector>

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

NyxFS_Error_Info nyxfs_enstructure(nyxfs_arena_t *arena, nyxfs_arena_t *s_tab, nyxfs_node_t *node)
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
      }
      else
      {
        rnode = nyxfs_node_create_child(node, arena);
      }
      if (rnode == nullptr)
      {
        return nyxfs_err_node_creation_failure;
      }
      rnode->type = nyxfs_node_raw;
      rnode->data.raw_data.text_offset = cx;
      nyxfs_enstructure(arena, s_tab, rnode);
    }
  }
  return nyxfs_okay;
}