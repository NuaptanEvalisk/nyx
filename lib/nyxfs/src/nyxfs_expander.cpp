//
// Created by felix on 2/23/26.
//

// we expand namespace to get subspace/files, etc.

#include "nyxfs_internals.hpp"
#include <filesystem>
#include <fstream>

bool nyxfs_delazy_node(nyxfs_node_t *node)
{
  if (node->first_child != nullptr && node->first_child->type == nyxfs_node_lazy)
  {
    node->first_child = nullptr;
    return true;
  }
  return false;
}

static inline __attribute__((always_inline))
std::string fast_read(const std::filesystem::path& path)
{
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  const std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::string buffer(size, '\0');
  if (file.read(&buffer[0], size))
  {
    return buffer;
  }
  return "";
}

NyxFS_Error_Info nyxfs_expand_ns_file(nyxfs_arena_t* arena, nyxfs_arena_t* s_tab, nyxfs_node_t* node, const bool lazy,
                                      const std::filesystem::path* p)
{
  const std::string contents = fast_read(*p);
  nyxfs_node_t *c = nyxfs_node_create_child(node, arena);
  if (c == nullptr)
  {
    return nyxfs_err_node_creation_failure;
  }
  c->type = nyxfs_node_raw;
  c->data.raw_data.text_offset = static_cast<char*>(nyxfs_symbol_tab_register(contents.c_str(), s_tab));
  if (!lazy)
  {
    return nyxfs_expand_raw(arena, s_tab, c, false);
  }
  return nyxfs_okay;
}

NyxFS_Error_Info nyxfs_expand_ns_dir(nyxfs_arena_t* arena, nyxfs_arena_t* s_tab, nyxfs_node_t* node, const bool lazy,
                                     const std::filesystem::path* p)
{
  for (const auto& entry : std::filesystem::directory_iterator(*p))
  {
    nyxfs_node_t* c = nyxfs_node_create_child(node, arena);
    if (c == nullptr)
    {
      return nyxfs_err_node_creation_failure;
    }
    c->type = nyxfs_node_namespace;
    auto abs_path = std::filesystem::absolute(entry.path());
    const char* str = abs_path.c_str();
    c->data.namespace_data.symb_offset = static_cast<char*>(nyxfs_symbol_tab_register(str, s_tab));
    if (lazy)
    {
      nyxfs_node_t* l = nyxfs_node_create_first_child(c, arena);
      if (l == nullptr)
      {
        return nyxfs_err_node_creation_failure;
      }
      l->type = nyxfs_node_lazy;
    }
    else
    {
      if (const NyxFS_Error_Info e = nyxfs_expand_namespace(arena, s_tab, c, false); e != nyxfs_okay)
      {
        return e;
      }
    }
  }
  return nyxfs_okay;
}

NyxFS_Error_Info nyxfs_expand_namespace(nyxfs_arena_t* arena, nyxfs_arena_t* s_tab, nyxfs_node_t* node, const bool lazy)
{
  // in namespace data:  char *symb_offset; having `\0`, do not need length.
  // the symbol should be absolute path of namespace.
  nyxfs_delazy_node(node);
  if (node->first_child != nullptr)
  {
    for (nyxfs_node_t *c = node->first_child; c != nullptr; c = c->next_sibling)
    {
      if (const NyxFS_Error_Info e = nyxfs_expand_node(arena, s_tab, c, false); e != nyxfs_okay)
      {
        return e;
      }
    }
    return nyxfs_okay;
  }

  char* symb = node->data.namespace_data.symb_offset;
  const std::filesystem::path p(symb);
  if (!std::filesystem::exists(p))
  {
    return nyxfs_err_file;
  }
  if (std::filesystem::is_directory(p))
  {
    return nyxfs_expand_ns_dir(arena, s_tab, node, lazy, &p);
  }
  return nyxfs_expand_ns_file(arena, s_tab, node, lazy, &p);
}

NyxFS_Error_Info nyxfs_expand_node(nyxfs_arena_t* arena, nyxfs_arena_t* symb, nyxfs_node_t* node, const bool lazy)
{
  if (arena == nullptr || node == nullptr || symb == nullptr)
  {
    return nyxfs_err_null_check;
  }

  // what sorts of nodes are expandable? namespace and raw. when you expand raw, parsing is conducted
  // and more sophisticated structures are constructed.
  switch (node->type)
  {
    case nyxfs_node_namespace:
      return nyxfs_expand_namespace(arena, symb, node, lazy);
    case nyxfs_node_raw:
      return nyxfs_expand_raw(arena, symb, node, lazy);
    default:
      break;
  }
  return nyxfs_okay;
}

// ultimately this will remove the raw node, so we will create siblings to raw node first, and then
// mark the first sibling as raw's parent's first child. since raw node will be destroyed, it is safe
// to modify things in its s_tab, like replacing some \n with \0 to split it into blocks.
NyxFS_Error_Info nyxfs_expand_raw(nyxfs_arena_t* arena, nyxfs_arena_t* s_tab, nyxfs_node_t* node, bool lazy)
{
  if (lazy)
  {
    return nyxfs_okay;
  }
}