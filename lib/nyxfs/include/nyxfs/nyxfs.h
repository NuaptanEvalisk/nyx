//
// Created by felix on 2/21/26.
//

#ifndef NYX_NYXFS_H
#define NYX_NYXFS_H

#include "nyxfs_types.h"
#include "nyxfs_err_list.h"

NyxFS_Error_Info nyxfs_expand_node(nyxfs_arena_t* arena, nyxfs_arena_t* symb, nyxfs_node_t* node, bool lazy);

#endif // NYX_NYXFS_H
