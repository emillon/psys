#ifndef _VFS_H
#define _VFS_H 1
#include "file_structs.h"

#define DTABLESIZE  20

struct fs_node;

struct file_descriptor {
    int present;            /* = 0 if this does not describe a valid entry */
    struct fs_node* node;
    unsigned long offset;
    /* Maybe add mode (RO/WO/RW), etc */
};

/* POSIX says : */
typedef int (*read_type_t) (struct fs_node* node,
                            unsigned long offset,
                            unsigned long length,
                            char* buffer);
typedef int (*write_type_t)(struct fs_node* node,
                            unsigned long offset,
                            unsigned long length,
                            const char* buffer);
typedef void (*open_type_t) (struct fs_node*);
typedef void (*close_type_t)(struct fs_node*);
typedef int  (*readdir_type_t) (struct fs_node*, struct dirent *, int n);
typedef struct fs_node * (*finddir_type_t) (struct fs_node*, const char *name);

typedef struct fs_node {
    char                name[FS_NAME_MAX];
    enum fs_node_type   type;                   /* File, directory, ... */
    int                 inode;
    size_t              size;

/* R/W functions */
    read_type_t         read;
    write_type_t        write;
    open_type_t         open;
    close_type_t        close;
    readdir_type_t      readdir;
    finddir_type_t      finddir;

    struct fs_node *    parent;
} fs_node_t;

void vfs_init(void);

/* Return node for '/'. Used to initialize working directory */
struct fs_node* vfs_root(void);

/* Provided for other modules */
void rootfs_setup_node (int inode, int parent_inode,
    const char* name, enum fs_node_type type);

void vfs_node_register_callbacks (
    int                 inode,
    read_type_t         read,
    write_type_t        write,
    open_type_t         open,
    close_type_t        close,
    readdir_type_t      readdir,
    finddir_type_t      finddir);

fs_node_t* vfs_path_resolution(const char* name);

struct fs_node *procfs_new_node        (int pid);
struct fs_node *procfs_new_state_node  (int pid);
struct fs_node *procfs_new_parent_node (int pid);
struct fs_node *procfs_new_name_node   (int pid);

#endif /* vfs.h */
