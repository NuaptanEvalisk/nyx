//
// Created by felix on 2/22/26.
//

#include "nyxfs_internals.hpp"
#include <cstdlib>
#include <cstring>

// 8-alignment, e.g. 10 |-> 16
#define NYXFS_ALIGN8(size) (((size) + 7) & ~7)

NyxFS_Error_Info nyxfs_make_arena(nyxfs_arena_t *arena)
{
  if (arena == nullptr || arena->size == 0)
  {
    return nyxfs_err_invalid_arena;
  }

  arena->beginning = aligned_alloc(8, arena->size);
  if (arena->beginning == nullptr)
  {
    return nyxfs_err_failed_malloc;
  }
  memset(arena->beginning, 0, arena->size);
  arena->top = arena->beginning;
  return nyxfs_okay;
}

void *nyxfs_arena_alloc(nyxfs_arena_t *arena, size_t size)
{
  if (arena == nullptr)
  {
    return nullptr;
  }
  const size_t occupied = static_cast<char *>(arena->top) -
    static_cast<char *>(arena->beginning);
  if (occupied + NYXFS_ALIGN8(size) > arena->size)
  {
    return nullptr;
  }
  void *t = arena->top;
  arena->top = static_cast<void *>(static_cast<char *>(arena->top) + NYXFS_ALIGN8(size));
  return t;
}

// perhaps we will implement gc some time, but not now, definitely.
void *nyxfs_arena_free_r(nyxfs_arena_t *arena, const size_t s)
{
  if (arena == nullptr || arena->size < s)
  {
    return nullptr;
  }
  arena->top = static_cast<void *>(static_cast<char *>(arena->top) - s);
  return arena->top;
}

void *nyxfs_arena_free_rp(nyxfs_arena_t *arena, void *ptr)
{
  if (arena == nullptr || ptr == nullptr)
  {
    return nullptr;
  }
  const size_t s = static_cast<char *>(arena->top) - static_cast<char *>(ptr);
  return nyxfs_arena_free_r(arena, s);
}

void *nyxfs_arena_next(const nyxfs_arena_t *arena, void *ptr, const size_t s)
{
  if (arena == nullptr || ptr == nullptr || ptr > arena->top)
  {
    return nullptr;
  }
  char *p = static_cast<char *>(ptr) + NYXFS_ALIGN8(s);
  if (p - static_cast<char *>(arena->beginning) > arena->size)
  {
    return nullptr;
  }
  return p;
}

__attribute__((always_inline))
void *nyxfs_arena_next_fast(void *ptr, const size_t s)
{
  return static_cast<char *>(ptr) + NYXFS_ALIGN8(s);
}

// no check would be conducted for the following functions for we are concerned
// with their speed. the following function must be called after filling data
// into the arena manually.
void *nyxfs_arena_pad8(nyxfs_arena_t *arena)
{
  const size_t u = static_cast<char *>(arena->top) - static_cast<char *>(arena->beginning);
  arena->top = static_cast<void *>(static_cast<char *>(arena->top) + u);
  return arena->top;
}

void *nyxfs_symbol_tab_register(const char *str, nyxfs_arena_t *arena)
{
  if (str == nullptr || arena == nullptr)
  {
    return nullptr;
  }
  const size_t len = strlen(str);
  const auto s = static_cast<char *>(nyxfs_arena_alloc(arena, len + 1));
  if (s == nullptr)
  {
    return nullptr;
  }
  memcpy(s, str, len + 1);
  return s;
}

nyxfs_node_t *nyxfs_node_create_first_child(nyxfs_node_t *n, nyxfs_arena_t *arena)
{
  if (n == nullptr || arena == nullptr)
  {
    return nullptr;
  }
  const auto f = static_cast<nyxfs_node_t *>(nyxfs_arena_alloc(arena, sizeof(nyxfs_node_t)));
  if (f == nullptr)
  {
    return nullptr;
  }
  f->parent = n;
  n->first_child = f;
  f->next_sibling = nullptr;
  f->first_child = nullptr;
  return f;
}

nyxfs_node_t *nyxfs_node_create_sibling(nyxfs_node_t *n, nyxfs_arena_t *arena)
{
  if (n == nullptr || arena == nullptr)
  {
    return nullptr;
  }
  const auto s = static_cast<nyxfs_node_t *>(nyxfs_arena_alloc(arena, sizeof(nyxfs_node_t)));
  if (s == nullptr)
  {
    return nullptr;
  }
  s->parent = n->parent;
  s->first_child = nullptr;
  s->next_sibling = nullptr;
  n->next_sibling = s;
  return s;
}

nyxfs_node_t *nyxfs_node_create_child(nyxfs_node_t *n, nyxfs_arena_t *arena)
{
  if (n == nullptr || arena == nullptr)
  {
    return nullptr;
  }
  if (n->first_child == nullptr)
  {
    return nyxfs_node_create_first_child(n, arena);
  }
  return nyxfs_node_create_sibling(n->first_child, arena);
}

nyxfs_node_t *nyxfs_next_child(const nyxfs_node_t *n, const nyxfs_node_t *cur)
{
  if (n == nullptr)
  {
    return nullptr;
  }
  if (cur == nullptr)
  {
    return n->first_child;
  }
  return cur->next_sibling;
}

__attribute__((always_inline))
void *nyxfs_token_data_begin(nyxfs_token_t *token)
{
  return token + 1;
}

__attribute__((always_inline))
nyxfs_token_t *nyxfs_token_next(nyxfs_token_t *token)
{
  auto t = reinterpret_cast<char *>(token + 1);
  t += token->data_length;
  return reinterpret_cast<nyxfs_token_t *>(t);
}
