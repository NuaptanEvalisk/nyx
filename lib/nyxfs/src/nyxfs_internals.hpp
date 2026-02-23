//
// Created by felix on 2/21/26.
//

#ifndef NYX_NYXFS_INTERNALS_HPP
#define NYX_NYXFS_INTERNALS_HPP

#include "nyxfs/nyxfs.h"

NyxFS_Error_Info nyxfs_make_arena(nyxfs_arena_t *arena);

void *nyxfs_arena_alloc(nyxfs_arena_t *arena, size_t size);

void *nyxfs_arena_free_r(nyxfs_arena_t *arena, size_t s);

void *nyxfs_arena_free_rp(nyxfs_arena_t *arena, void *ptr);

void *nyxfs_arena_next(const nyxfs_arena_t *arena, void *ptr, size_t s);

void *nyxfs_arena_next_fast(void *ptr, size_t s);

void *nyxfs_arena_append1(nyxfs_arena_t *arena, char c);

void *nyxfs_arena_pad8(nyxfs_arena_t *arena);

void *nyxfs_symbol_tab_register(const char *str, nyxfs_arena_t *arena);

nyxfs_node_t *nyxfs_node_create_first_child(nyxfs_node_t *n, nyxfs_arena_t *arena);

nyxfs_node_t *nyxfs_node_create_sibling(nyxfs_node_t *n, nyxfs_arena_t *arena);

nyxfs_node_t *nyxfs_node_create_child(nyxfs_node_t *n, nyxfs_arena_t *arena);

nyxfs_node_t *nyxfs_next_child(const nyxfs_node_t *n, const nyxfs_node_t *cur);

void *nyxfs_token_data_begin(nyxfs_token_t *token);

nyxfs_token_t *nyxfs_token_next(nyxfs_token_t *token);

NyxFS_Error_Info nyxfs_expand_namespace(nyxfs_arena_t* arena, nyxfs_arena_t* s_tab, nyxfs_node_t* node, bool lazy);

NyxFS_Error_Info nyxfs_expand_raw(nyxfs_arena_t* arena, nyxfs_arena_t* s_tab, nyxfs_node_t* node, bool lazy);

NyxFS_Error_Info nyxfs_enstructure(nyxfs_arena_t *arena, nyxfs_node_t *node);

#endif // NYX_NYXFS_INTERNALS_HPP
