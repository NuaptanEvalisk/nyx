//
// Created by felix on 2/21/26.
//

#ifndef NYX_NYXFS_ERR_LIST_H
#define NYX_NYXFS_ERR_LIST_H

#define NYXFS_ERROR_LIST(X) \
X(nyxfs_okay) \
X(nyxfs_err_null_check) \
X(nyxfs_err_file) \
X(nyxfs_err_failed_alloc)

#define GENERATE_ENUM(ENUM) ENUM,
enum NyxFS_Error_Info
{
  NYXFS_ERROR_LIST(GENERATE_ENUM)
};
#undef GENERATE_ENUM

#define GENERATE_STRING(STRING) case STRING: return #STRING;

// if no static, would cause C linker error
static inline const char* nyxfs_explain_err(const enum NyxFS_Error_Info err)
{
  switch (err)
  {
    NYXFS_ERROR_LIST(GENERATE_STRING)
    default: return "nyxfs_err_unknown";
  }
}

#undef GENERATE_STRING


#endif // NYX_NYXFS_ERR_LIST_H
