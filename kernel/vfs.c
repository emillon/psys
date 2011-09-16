#include "vfs.h"
#include "context.h"
#include "syscalls.h"
#include "console.h"

#include "mem.h"
#include "sys.h"
#include "string.h"
#include "stdlib.h"

/* VFS wrappers (make use of nodes callbacks) */

static int open_fs (fs_node_t *node)
{
    if (node->open) {
        node->open(node);
    }
    return 0;
}

static int close_fs (fs_node_t *node)
{
    if (node->close) {
        node->close(node);
    }
    return 0;
}

static int read_fs (fs_node_t *node, unsigned long offset,
                                     unsigned long len, char* buf)
{
    if(node->read) {
        return node->read(node, offset, len, buf);
    }
    return 0;
}

static int write_fs (fs_node_t *node, unsigned long offset, unsigned long len,
                                                     const char* buf)
{
    if(node->write) {
        return node->write(node, offset, len, buf);
    }
    return 0;
}

static int readdir_fs (fs_node_t *node, struct dirent* dirp, int n)
{
    if (node->readdir != NULL && node->type == FS_DIR) {
        return node->readdir(node, dirp, n);
    }
    return -1;
}

static struct fs_node* finddir_fs(fs_node_t *node, const char* name)
{
    if (node->finddir !=NULL && node->type == FS_DIR) {
        if(!strcmp(name, "..")) return node->parent;
        return node->finddir(node, name);
    }
    return NULL;
}

/* Allocate/free file descriptors for the current process */

static int fd_alloc(fs_node_t* node)
{
    int i;
    for(i=0;i<DTABLESIZE;i++) {
        if (tab_proc[pid_cour-1].file_desc[i].present==0) {
            tab_proc[pid_cour-1].file_desc[i].present = 1;
            tab_proc[pid_cour-1].file_desc[i].node    = node;
            return i;
        }
    }
    return -1;  /* No room left */
}

static int fd_free(int fd)
{
    if(tab_proc[pid_cour-1].file_desc[fd].present == 0)
        return -1;
    tab_proc[pid_cour-1].file_desc[fd].present = 0;
    tab_proc[pid_cour-1].file_desc[fd].node=(void*)0;
    return 0;
}

/* Path resolution : convert a path into a node */

/*
 * Parser : read next "filename" from "name" starting at offset
 * "*offset_name". Return 1 if there is something left to read.
 */
static int vfs_next_filename(char* filename, const char* name, int *offset_name)
{
    int i = 0;
    while(1) {
        char c = (filename[i]=name[*offset_name]);
        if(c=='\0') {
            return 1;
        }
        if(c=='/') {
            filename[i]='\0';
            break;
        }
        i++;
        (*offset_name)++;
    }
    return 0;
}

static fs_node_t* fs_root;

/* Resolution itself */
fs_node_t* vfs_path_resolution(const char* name)
{
    fs_node_t *current_node, *next_node;
    int offset_name = 0;
    if(name[0] != '/') {
        /* Relative path */
        current_node = tab_proc[pid_cour-1].working_dir;
        /* . = self */
        if(!strncmp(name,".",FS_NAME_MAX))
            return current_node;
        /* .. = parent */
        if(!strncmp(name,"..",FS_NAME_MAX))
            return current_node->parent;

        offset_name=-1; /* do not skip first character */
    } else {
        /* Absolute path */
        current_node = fs_root;
        if(name[1] == '\0'){
            return fs_root;
        }
    }
    while(1) {
        char filename [FS_NAME_MAX];
        filename[0]='\0';
        offset_name++;
        int end = vfs_next_filename(filename, name, &offset_name);

        /* ok, is there "filename" in current_directory ? */
        if((next_node = finddir_fs(current_node, filename)) == NULL) {
            return NULL; /* No such file or directory */
        }
        current_node = next_node;
        if(end)
            return current_node;
    }

    return NULL;
}

/* Syscalls */

int read(int fd, char* buf, unsigned long len)
{
    buf = check_pointer(buf, SYSCALL_READ);
    int bytes_read=read_fs(tab_proc[pid_cour-1].file_desc[fd].node,
                           tab_proc[pid_cour-1].file_desc[fd].offset, len, buf);
    tab_proc[pid_cour-1].file_desc[fd].offset += bytes_read;
    return bytes_read;
}

int write(int fd, const char* buf, unsigned long len)
{
    //buf = check_pointer(buf);
    return write_fs(tab_proc[pid_cour-1].file_desc[fd].node,
                    tab_proc[pid_cour-1].file_desc[fd].offset, len, buf);
}

int readdir(int fd, struct dirent *dirp, int n)
{
    dirp = check_pointer(dirp, SYSCALL_READDIR);
    return readdir_fs(tab_proc[pid_cour-1].file_desc[fd].node, dirp, n);
}

int close (int fd)
{
    if(close_fs(tab_proc[pid_cour-1].file_desc[fd].node)) return -1;
    return (fd_free(fd));
}

int open(const char *name)
{
    int fd = -1;
    fs_node_t * node = vfs_path_resolution(name);
    if(node == NULL)
        return -1;
    if(open_fs(node)<0) return -1;
    fd = fd_alloc(node);
    tab_proc[pid_cour-1].file_desc[fd].offset = 0;
    return fd;
}

int fstat (int fd, struct stat *s)
{
    if(tab_proc[pid_cour-1].file_desc[fd].present==0)
        return -1;
    s->type =tab_proc[pid_cour-1].file_desc[fd].node->type;
    s->inode=tab_proc[pid_cour-1].file_desc[fd].node->inode;
    s->size =tab_proc[pid_cour-1].file_desc[fd].node->size;
    strncpy(s->name,tab_proc[pid_cour-1].file_desc[fd].node->name,FS_NAME_MAX);
    return 0;
}

char* getcwd(char* buf, size_t len)
{
    buf = check_pointer(buf, SYSCALL_GETCWD);
    if(buf == NULL) return NULL;
    /* XXX does not return full dir, should use vfs_canonicalize() */
    strncpy(buf, tab_proc[pid_cour-1].working_dir->name, len);
    return buf;
}

int chdir (const char *path)
{
    path = check_pointer(path, SYSCALL_CHDIR);
    fs_node_t *node = vfs_path_resolution(path);
    if(node == NULL) {
        return -1;
    }
    if (node->type!=FS_DIR)
        return -1;
    tab_proc[pid_cour-1].working_dir = node;
    return 0;
}

/* Misc. filesystems */

/* Rootfs */
struct rootfs_entry {
    int present;
    fs_node_t node;
};

#define ROOTFS_ENTRIES 64
static struct rootfs_entry rootfs_entries [ROOTFS_ENTRIES];

static fs_node_t* rootfs_finddir (fs_node_t *node, const char* name)
{
    int i;
    for(i=0;i<ROOTFS_ENTRIES;i++) {
        if    (         (rootfs_entries[i].node.parent == node)
            && (!strncmp(rootfs_entries[i].node.name  ,name,  FS_NAME_MAX))
            &&          (rootfs_entries[i].present != 0)) {
                 return &rootfs_entries[i].node;
        }
    }
    return NULL;    /* Not found */
}

static int rootfs_readdir (fs_node_t *node, struct dirent* dirp, int n){
    int found=0;
    if (n<0) return -1;
    /* Find (n+1)th node whose parent is "node" */
    int i;
    for(i=0;i<ROOTFS_ENTRIES;i++) {
        if(rootfs_entries[i].present == 0)    continue;
        if(rootfs_entries[i].node.parent  != node) continue;
        if(n==found) {
            strncpy(dirp->name, rootfs_entries[i].node.name, FS_NAME_MAX);
            dirp->inode=i;
            return 1;
        }
        found++;
    }
    return 0;
}

static fs_node_t *fs_root = &rootfs_entries[0].node;

void rootfs_setup_node (int offset, int parent,
    const char* name, enum fs_node_type type)
{
    rootfs_entries[offset].present      = 1;
    rootfs_entries[offset].node.readdir = rootfs_readdir;
    rootfs_entries[offset].node.finddir = rootfs_finddir;
    rootfs_entries[offset].node.read    = NULL;
    rootfs_entries[offset].node.write   = NULL;
    rootfs_entries[offset].node.open    = NULL;
    rootfs_entries[offset].node.close   = NULL;
    rootfs_entries[offset].node.inode   = offset;
    rootfs_entries[offset].node.type    = type;
    if(parent<0) {
        rootfs_entries[offset].node.parent   = NULL;
    } else {
        rootfs_entries[offset].node.parent   = &rootfs_entries[parent].node;
    }
    strncpy(rootfs_entries[offset].node.name, name, FS_NAME_MAX);
}

void vfs_node_register_callbacks (
    int                 inode,
    read_type_t         read,
    write_type_t        write,
    open_type_t         open,
    close_type_t        close,
    readdir_type_t      readdir,
    finddir_type_t      finddir)
{
    rootfs_entries[inode].node.read    = read;
    rootfs_entries[inode].node.write   = write;
    rootfs_entries[inode].node.open    = open;
    rootfs_entries[inode].node.close   = close;
    if (readdir) rootfs_entries[inode].node.readdir = readdir;
    if (finddir) rootfs_entries[inode].node.finddir = finddir;
}

fs_node_t * vfs_procfs_root(void)
{
    return &rootfs_entries[1].node;
}

/******************************* FS Callbacks ********************************/

static int dev_answer_read(fs_node_t *node, unsigned long offset,
        unsigned long len, char* buf)
{
    memset(buf, 42, len);
    return len;
}

static int dev_zero_read(fs_node_t *node, unsigned long offset,
        unsigned long len, char *buf)
{
    memset(buf, 0, len);
    return len;
}
static char* kikoobuf;
static void kikoo_open(fs_node_t *node)
{
    static const char* kikoomsg = "Kikooooooo !\nLool ! ptdr\n";
    kikoobuf = mem_alloc(30);
    strcpy(kikoobuf, kikoomsg);
}
static void kikoo_close(fs_node_t *node)
{
    mem_free(kikoobuf, 30);
}
static int kikoo_read(fs_node_t *node, unsigned long offset,
        unsigned long len, char *buf)
{
    return memread(kikoobuf, 26, offset, buf, len);
}
static int procfs_readdir(struct fs_node* node, struct dirent *dirp, int n)
{
    int found=0;
    if (n<0) return -1;
    if (node != vfs_procfs_root()) return -1;
    /* Find (n+1)th node */
    int i;
    for(i=0;i<NBPROC;i++) {
        if(tab_proc[i].valide == 0) continue;
        if(n==found) {
            snprintf(dirp->name, FS_NAME_MAX, "%d", i+1);
            dirp->inode=i;
            return 1;
        }
        found++;
    }
    return 0;
}
static struct fs_node * procfs_finddir (struct fs_node* node, const char *name)
{
    int pid = strtol(name, NULL, 10);
    if(tab_proc[pid-1].valide == 0) return NULL;
    return tab_proc[pid-1].procfs_node;
}

/****************************End of  FS Callbacks ****************************/

void vfs_init(void)
{
    /* Initial rootfs */
    rootfs_setup_node(              0, -1, "root",     FS_DIR);
        rootfs_setup_node(          1, 0, "proc",      FS_DIR);
        rootfs_setup_node(          2, 0, "sem",       FS_DIR);
        rootfs_setup_node(          3, 0, "sys",       FS_DIR);
            rootfs_setup_node(      4, 3, "console",   FS_DIR);
                rootfs_setup_node(  5, 4, "blink",     FS_FILE);
                rootfs_setup_node(  6, 4, "fg",        FS_FILE);
                rootfs_setup_node(  7, 4, "bg",        FS_FILE);
                rootfs_setup_node( 13, 4, "echo",      FS_FILE);
        rootfs_setup_node(          8, 0, "dev",       FS_DIR);
            rootfs_setup_node(      9, 8, "null",      FS_FILE);
            rootfs_setup_node(     10, 8, "zero",      FS_FILE);
            rootfs_setup_node(     11, 8, "answer",    FS_FILE);
            rootfs_setup_node(     16, 8, "console",   FS_FILE);
            rootfs_setup_node(     17, 8, "vc",        FS_DIR);
        rootfs_setup_node(         12, 0, "kikoo",     FS_FILE);
        rootfs_setup_node(         14, 0, "prog",      FS_DIR);

    /* /dev/answer */
    rootfs_entries[11].node.read    = dev_answer_read;

    /* /dev/zero */
    rootfs_entries[10].node.read    = dev_zero_read;

    /* /kikoo */
    rootfs_entries[12].node.open    = kikoo_open;
    rootfs_entries[12].node.read    = kikoo_read;
    rootfs_entries[12].node.close   = kikoo_close;

    /* /proc */
    rootfs_entries[1].node.readdir  = procfs_readdir;
    rootfs_entries[1].node.finddir  = procfs_finddir;

    console_vfs_init();
}

struct fs_node* vfs_root(void)
{
    return fs_root;
}

static int procfs_sub_readdir (struct fs_node *node, struct dirent *dirp, int n)
{
    static const char* directories[] = {
        "state",
        "parent",
        "name"
    };
    if (n<0)  return -1;
    if (n>3)  return -1;
    if( n==3) return 0;
    strncpy(dirp->name, directories[n], FS_NAME_MAX);
    dirp->inode = (n+1)*1000+node->inode;
    return 1;
}

static struct fs_node *procfs_sub_finddir (fs_node_t *node, const char *name)
{
    int pid = node->inode - 200;
    if        (!strncmp(name, "state",  FS_NAME_MAX)) {
        return tab_proc[pid-1].vfs_state_node;
    } else if (!strncmp(name, "parent", FS_NAME_MAX)) {
        return tab_proc[pid-1].vfs_parent_node;
    } else if (!strncmp(name, "name",   FS_NAME_MAX)) {
        return tab_proc[pid-1].vfs_name_node;
    }
    return NULL;
}

struct fs_node *procfs_new_node(int pid)
{
    struct fs_node *node = mem_alloc(sizeof(struct fs_node));
    snprintf(node->name, FS_NAME_MAX, "%d", pid);
    node->type = FS_DIR;
    node->inode = 200+pid;
    node->readdir = procfs_sub_readdir;
    node->finddir = procfs_sub_finddir;
    return node;
}

#define _CONTEXT_STATE_CASE(S) case S: state=#S; break;
static int procfs_state_read (fs_node_t *node, unsigned long offset,
                                unsigned long length, char* buf)
{
    char* state;
    int pid = node->inode - 1200;
    switch(tab_proc[pid-1].state){
        _CONTEXT_STATE_CASE(STATE_ACTIVE)
        _CONTEXT_STATE_CASE(STATE_ACTIVABLE)
        _CONTEXT_STATE_CASE(STATE_WAIT_SEM)
        _CONTEXT_STATE_CASE(STATE_WAIT_IO)
        _CONTEXT_STATE_CASE(STATE_WAIT_CHILD)
        _CONTEXT_STATE_CASE(STATE_ASLEEP)
        _CONTEXT_STATE_CASE(STATE_ZOMBIE)
        _CONTEXT_STATE_CASE(STATE_DEAD)
    }
    return memread(state, strlen(state), offset, buf, length);
}

struct fs_node *procfs_new_state_node(int pid)
{
    struct fs_node *node = mem_alloc(sizeof(struct fs_node));
    strncpy(node->name, "state" , FS_NAME_MAX);
    node->type = FS_FILE;
    node->inode = 1200 + pid;
    node->read = procfs_state_read;
    return node;
}

static int procfs_parent_read (fs_node_t *node, unsigned long offset,
                                unsigned long length, char* buf)
{
    char pbuf[32];
    int pid = node->inode - 2200;
    int ppid = tab_proc[pid-1].pid_parent;
    snprintf(pbuf, 32, "%d", ppid);
    return memread(pbuf, strlen(pbuf), offset, buf, length);
}

struct fs_node *procfs_new_parent_node(int pid)
{
    struct fs_node *node = mem_alloc(sizeof(struct fs_node));
    strncpy(node->name, "parent" , FS_NAME_MAX);
    node->type = FS_FILE;
    node->inode = 2200 + pid;
    node->read = procfs_parent_read;
    return node;
}

static int procfs_name_read (fs_node_t *node, unsigned long offset,
                                unsigned long length, char* buf)
{
    int pid = node->inode - 3200;
    return memread(tab_proc[pid-1].name, strlen(tab_proc[pid-1].name),
                        offset, buf, length);
}

struct fs_node *procfs_new_name_node(int pid)
{
    struct fs_node *node = mem_alloc(sizeof(struct fs_node));
    strncpy(node->name, "name" , FS_NAME_MAX);
    node->type = FS_FILE;
    node->inode = 3200 + pid;
    node->read = procfs_name_read;
    return node;
}

