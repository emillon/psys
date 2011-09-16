#ifndef _FILE_STRUCTS_H
#define _FILE_STRUCTS_H 1
#include "types.h"

#define FS_NAME_MAX 128

enum fs_node_type {
    FS_FILE,
    FS_DIR
};

struct dirent {
    char name[FS_NAME_MAX];
    int inode;
};

struct stat {
    char name[FS_NAME_MAX];
    enum fs_node_type type;
    int inode;
    size_t size;
};

#endif /* file_structs.h */
