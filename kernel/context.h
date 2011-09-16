#ifndef _CONTEXT_H
#define _CONTEXT_H 1

#include "queue.h"
#include "list.h"
#include "syscalls.h"
#include "vfs.h"


//*********************************Max
#define NBSIG 32
//********************************\Max

#define NBPROC 20

#define MAXPRIO 256


enum proc_state {
    STATE_ACTIVE,
    STATE_ACTIVABLE,
    STATE_WAIT_SEM,
    STATE_WAIT_IO,
    STATE_WAIT_CHILD,
    STATE_ASLEEP,
    STATE_ZOMBIE,
    STATE_DEAD
};
//*********************************Max
void sig_default();
//********************************\Max

typedef struct {
    unsigned long           ebp;
    unsigned long           eax;
    unsigned long           ebx;
    unsigned long           ecx;
    unsigned long           edx;
    unsigned long           edi;
    unsigned long           esi;
} registers;

/* Do not move fields ! (offsets are hardcoded in assembly) */
typedef struct {
    unsigned long           esp_kernel;             /* ESP kernel stack */
    unsigned long *         kernel_stack;           /* @ kernel stack */
    unsigned long           esp_user;               /* ESP user stack */
    unsigned long *         user_stack;             /* @ user stack */
    unsigned long           user_stack_size;        /* user stack size
                                                       in unsigned long */
    unsigned long           kernel_stack_size;      /* kernel stack size
                                                       in unsigned long */
    registers               regs;                   /* registers */
    int                     pid;                    /* process PID */
    int                     priority;               /* process priority */
    int                     pid_parent;             /* process father */
    int                     pid_wait;               /* child waited */
    enum proc_state         state;                  /* process state */
    list_t                  children;               /* children list */
    link                    activable;              /* process activable list */
    int                     killed;                 /* process was killed */
    int                     val_ret;                /* return value */
    int                     valide;                 /* PID used */
    char *                  name;                   /* process name */
    unsigned long           timer;                  /* awaking time */
    struct file_descriptor  file_desc[DTABLESIZE];  /* file descriptors */
    struct fs_node *        working_dir;            /* working directory */
    struct fs_node *        procfs_node;            /* describing node */
    struct fs_node *        vfs_state_node;         /* node for pid/state */
    struct fs_node *        vfs_parent_node;        /* node for pid/parent */
    struct fs_node *        vfs_name_node;          /* node for pid/name */
    int    sid_wait;                                /* SID waited */
    int    sid_deleted;
    int    sid_reseted;
//*********************************Max
    void                    (*sigmap[NBSIG])(int);
    list_t                  sigs;
    unsigned long           edi;
    unsigned long           esi;
    unsigned long           ebp;
    unsigned long           edx;
    unsigned long           ecx;
    unsigned long           ebx;
    unsigned long           eax;
    unsigned long           ex_ebp;
    unsigned long           ad_iret;
    unsigned long           ad_proc;
    unsigned long           cs;
    unsigned long           eflags;
    unsigned long           esp;
    unsigned long           ss;

   unsigned int             cur_sig;
//********************************\Max
} context;

struct list_link proc_queue_activ;                  /* activable process list */

context tab_proc[NBPROC];                           /* array of 
                                                       process descriptors */

context kernel_context;                             /* kernel descriptor */

unsigned int pid_cour;                              /* current process */

char * adExit;                                      /* @ user terminaison
                                                       function */

/* Switch context from user process to another user context */
void context_switch (context* old, context* new);

/* Switch context from kernel to user context */
void _context_switch (context* new);

/* Scheduler */
void scheduler();

/* Create a context for a new user process */
int context_create_user(void* f, unsigned long stack_size, int priority,
                   const char * name, void * arg);

/* Create a context for a new kernel process */
int context_create_kernel(void* f, unsigned long stack_size, int priority,
                   const char * name, void * arg);

/* Tell a process it's ready to run */
int wakeup_process(int pid);

/* Called when IO is done (signal waiting process) */
void signal_new_io(int pid);

/* Initialize context support */
char * adSigReturn;

void context_init(void);

/* Free memory of dead process */
void garbage_collector();

#endif /* context.h */

