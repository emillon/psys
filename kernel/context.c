#include "context.h"
#include "queue.h"
#include "mem.h"
#include "user_stack_mem.h"
#include "list.h"
#include "sys.h"
#include "string.h"
#include "processor_structs.h"
#include "sig_init.h"

#include "signals.h"

extern void int32(void);

unsigned int pid_cour = 0;
unsigned int nb_proc_run = 0;

/* init activable process list */

LIST_HEAD(proc_queue_activ);

void scheduler() {
    int pid_prec;
    context * elem;

    /* kernel is running */
    if(pid_cour==0) {
        /* getting and removing activable process from list*/
        elem = queue_out(&proc_queue_activ, context, activable);
        /* there is an activable process in list */
        if(elem != 0) {
            pid_cour = elem->pid;
            tab_proc[pid_cour-1].state = STATE_ACTIVE;
            /* switching context */
            context_switch(&kernel_context, &(tab_proc[pid_cour-1]));
        }
    } 
    else {
        /* getting activable process from list */
        elem = queue_top(&proc_queue_activ, context, activable);

        /* there is an activable process */
        if(elem != 0) {
            /* activable process must be run */
            if(   tab_proc[pid_cour-1].state != STATE_ACTIVE
               || elem->priority >= tab_proc[pid_cour-1].priority) {

                /* remove process to run from activable process list */
                queue_del(elem, activable);

                /* current process active */
                if(tab_proc[pid_cour-1].state == STATE_ACTIVE) {
                    /* adding process to activable process list */
                    wakeup_process(pid_cour);
                }

                /* changing current process */
                pid_prec = pid_cour;
                pid_cour = elem->pid;
                tab_proc[pid_cour-1].state = STATE_ACTIVE;

                /* switching context */
                context_switch(&(tab_proc[pid_prec-1]),
                               &(tab_proc[pid_cour-1]));
            }
        }
        /* there is no activable process */
        else {
            /* current process not active */
            if(tab_proc[pid_cour-1].state != STATE_ACTIVE) {
                /* running kernel */
                pid_prec = pid_cour;
                pid_cour = 0;
                /* switching context */
                context_switch(&(tab_proc[pid_prec-1]),&kernel_context);
            }
        }
    }
}

__attribute__((deprecated))
int context_create_kernel(void* f, unsigned long stack_size, int priority,
                   const char *name, void * arg)
{
    int i;
    unsigned long *kernel_stack;

    if(f==NULL || name==NULL || priority<0) {
        printf("Incorrect parameter\n");
        return -1;
    }

    for(i=0; i<NBPROC; ++i) {
        if(tab_proc[i].valide != 1) {
            size_t size = stack_size * sizeof(unsigned long);
            kernel_stack      = (unsigned long*) mem_alloc(size);
            if(kernel_stack==0) {
                printf("Not enough memory\n");
                return -1;
            }
            tab_proc[i].esp_kernel =(unsigned long)&kernel_stack[stack_size-11];
            tab_proc[i].pid             = i+1;
            tab_proc[i].state           = STATE_ACTIVABLE;
            tab_proc[i].priority        = priority;
            tab_proc[i].kernel_stack_size      = stack_size;
            tab_proc[i].kernel_stack    = kernel_stack;
            tab_proc[i].pid_parent      = pid_cour;
            tab_proc[i].valide          = 1;
            tab_proc[i].killed          = 0;
            tab_proc[i].children        = list_new();

            tab_proc[i].name = mem_alloc(strlen(name)+sizeof('\0'));
            strcpy(tab_proc[i].name, name);

            tab_proc[i].working_dir = NULL;
            tab_proc[i].file_desc[0].present = 1;
            tab_proc[i].file_desc[0].node    = NULL;
            tab_proc[i].file_desc[1].present = 1;
            tab_proc[i].file_desc[1].node    = NULL;
            
            if(pid_cour != 0) {
                list_put_first (tab_proc[pid_cour-1].children, i+1);
            }
            else {
                list_put_first (kernel_context.children, i+1);
            } 

            kernel_stack[stack_size-1] = (unsigned long) arg;
            kernel_stack[stack_size-2] = (unsigned long) _exit_kernel;
            kernel_stack[stack_size-3] = (unsigned long) f;
            kernel_stack[stack_size-9] = (unsigned long)
                                                    &kernel_stack[stack_size-4];

            wakeup_process(i+1);

            return i+1;
        }
    }

    printf("Too many processes\n"); context_ps();
    return -1;
}

int context_create_user(void* f, unsigned long stack_size, int priority,
                   const char *name, void * arg)
{
    int i;
    unsigned long *kernel_stack, *user_stack;

    /* searching free PID */
    for(i=0; i<NBPROC; ++i) {
        /* free PID find */
        if(tab_proc[i].valide != 1) {
            /* memory stack allocation */
            size_t nb_elem_u = stack_size / sizeof(unsigned long);

            size_t nb_elem_k = 40000 / sizeof(unsigned long);
            kernel_stack = (unsigned long*) mem_alloc(40000);

            user_stack = (unsigned long*) user_stack_alloc(stack_size);
            
            /* memory error */
            if(kernel_stack==0 || user_stack==0) {
                return -1;
            }
            
            /* affecting process ESP register */
            tab_proc[i].esp_kernel = (unsigned long)
                                        &kernel_stack[nb_elem_k-8];
            tab_proc[i].esp_user = (unsigned long)
                                        &user_stack[nb_elem_u-2];
                                        
            /* affecting process properties */
            tab_proc[i].pid             = i+1;
            tab_proc[i].state           = STATE_ACTIVABLE;
            tab_proc[i].priority        = priority;
            tab_proc[i].user_stack_size = stack_size;
            tab_proc[i].kernel_stack_size = 40000;
            tab_proc[i].kernel_stack    = kernel_stack;
            tab_proc[i].user_stack      = user_stack;
            tab_proc[i].pid_parent      = pid_cour;
            tab_proc[i].killed          = 0;
            tab_proc[i].children        = list_new();
            tab_proc[i].sid_wait        = 0;
            tab_proc[i].sid_reseted     = 0;
            tab_proc[i].sid_deleted     = 0;
            if(tab_proc[i].children == NULL) {
                return -1;
            }

//*********************************Max
            for (int j=0; j<NBSIG; j++){
                tab_proc[i].sigmap[j]   = (void*) ad_sig_default;
            }
            tab_proc[i].sigs            = list_new();
            tab_proc[i].cur_sig         = 0;
//********************************\Max

            tab_proc[i].name = mem_alloc(strlen(name)+sizeof('\0'));
            strcpy(tab_proc[i].name, name);

            /* setup file descriptors */
            int fd;
            tab_proc[i].file_desc[0].present = 1;   /* Reserved (stdin)  */
            tab_proc[i].file_desc[1].node = vfs_path_resolution("/dev/kbd");
            tab_proc[i].file_desc[1].offset = 0;
            tab_proc[i].file_desc[1].present = 1;   /* Reserved (stdout)  */
            tab_proc[i].file_desc[1].node =
                (pid_cour == 0) ? vfs_path_resolution("/dev/vc/0")
                                      : tab_proc[pid_cour-1].file_desc[1].node;
            tab_proc[i].file_desc[1].offset = 0;
            for(fd=2;fd<DTABLESIZE;fd++) {
                tab_proc[i].file_desc[fd].present=0;
            }
            tab_proc[i].working_dir = vfs_root();
            tab_proc[i].procfs_node     = procfs_new_node        (i+1);
            tab_proc[i].vfs_state_node  = procfs_new_state_node  (i+1);
            tab_proc[i].vfs_parent_node = procfs_new_parent_node (i+1);
            tab_proc[i].vfs_name_node   = procfs_new_name_node   (i+1);
            
            /* attach process to his father */
            if(pid_cour != 0) {
                list_put_first (tab_proc[pid_cour-1].children, i+1);
            }
            else {
                list_put_first (kernel_context.children, i+1);
            }

            /* kernel stack initialization */
            kernel_stack[nb_elem_k-1]  = 0x4b;                      /* SS */
            kernel_stack[nb_elem_k-2]  = (unsigned long)
                                           &user_stack[nb_elem_u-2];/* ESP */
            kernel_stack[nb_elem_k-3]  = 0x202;                     /* EFLAGS */
            kernel_stack[nb_elem_k-4]  = 0x43;                      /* CS */
            kernel_stack[nb_elem_k-5]  = (unsigned long) f;         /* EIP */
            kernel_stack[nb_elem_k-6]  = (unsigned long) _iret;     /* @iret */
            kernel_stack[nb_elem_k-8] = (unsigned long)
                                           &kernel_stack[nb_elem_k-7];
                                                                    /* old EBP*/
            /* user stack initialization */
            user_stack[nb_elem_u-1] = (unsigned long) arg;          /* arg */
            user_stack[nb_elem_u-2] = (unsigned long) adExit;       /* @exit */

            tab_proc[i].valide          = 1;

            /* add process to activable process list */
            wakeup_process(i+1);

            return i+1;
        }
    }

    /* no free PID */
    return -1;
}

int wakeup_process(int pid)
{
    /* invalid PID */
    if(pid < 1 || pid > NBPROC || tab_proc[pid-1].valide!=1) {
        return -1;
    }

    /* change process state */
    tab_proc[pid-1].state = STATE_ACTIVABLE;
    
    /* adding process to activable process list */
    INIT_LINK(&(tab_proc[pid-1].activable));
    queue_add(&tab_proc[pid-1], &proc_queue_activ, context,
              activable, priority);

    return pid;
}

void signal_new_io(int pid)
{
    wakeup_process(pid);
}

void context_init(void)
{
    /* process descriptor array initialization */
    for(int i=0; i<NBPROC; ++i) {
        tab_proc[i].valide = 0;
    }
    /* clock activation */
    fill_interrupt(32, KERNEL_CS, int32, 0);

    /* kernel process initialization */
    kernel_context.state      = STATE_ACTIVE;
    kernel_context.killed     = 0;
    kernel_context.children   = list_new();
    kernel_context.valide     = 1;
    kernel_context.pid        = 0;
    kernel_context.pid_parent = 0;
    kernel_context.pid_wait   = 0;

    /* launching kernel process */
    pid_cour = 0;

    /* creation of the user terminaison function */
    adExit    =  (char *) user_stack_alloc(9*sizeof(char));
    memcpy(adExit,
                    "\x89\xc3"             /* mov %eax, %ebx */ 
                    "\xb8\x00\x00\x00\x00" /* mov $SYSCALL_EXIT, %eax */
                    "\xcd\x31",            /* int $0x31 */
            9);
    adExit[3] = SYSCALL_EXIT;

    adSigReturn    =  (char *) user_stack_alloc(9*sizeof(char));
    memcpy(adSigReturn,
                    "\x89\xc3"             /* mov %eax, %ebx */ 
                    "\xb8\x00\x00\x00\x00" /* mov $SYSCALL_EXIT, %eax */
                    "\xcd\x31",            /* int $0x31 */
            9);
    adSigReturn[3] = SYSCALL_SIG_RETURN;

    ad_sig_default =(char*) user_stack_alloc(1*sizeof(char));
    *ad_sig_default = 0xc3; /* ret */

    /* starting Init process */
    context_create_user ((void*) 0x1000000, STACK_SIZE, 3, "Init", (void*)0);
}

#define _CONTEXT_STATE_CASE(S) case S: return #S;
static const char* context_print_state (enum proc_state state)
{
    switch(state){
        _CONTEXT_STATE_CASE(STATE_ACTIVE)
        _CONTEXT_STATE_CASE(STATE_ACTIVABLE)
        _CONTEXT_STATE_CASE(STATE_WAIT_SEM)
        _CONTEXT_STATE_CASE(STATE_WAIT_IO)
        _CONTEXT_STATE_CASE(STATE_WAIT_CHILD)
        _CONTEXT_STATE_CASE(STATE_ASLEEP)
        _CONTEXT_STATE_CASE(STATE_ZOMBIE)
        _CONTEXT_STATE_CASE(STATE_DEAD)
    }
    BUG();
    return NULL;
}

void context_ps(void)
{
    int i;
    printf("PID       Name       Pri      State       PPID\n");
    printf("==============================================\n");
    for(i=0;i<NBPROC;i++) {
        if(!tab_proc[i].valide) continue;
        printf("%3d %16s %3d %16s  %3d\n",
            tab_proc[i].pid,
            tab_proc[i].name,
            tab_proc[i].priority,
            context_print_state(tab_proc[i].state),
            tab_proc[i].pid_parent
        );
    }
}

void garbage_collector() {
    for(int i=0; i<NBPROC; ++i) {
        if(tab_proc[i].valide == 1 && tab_proc[i].state == STATE_DEAD) {
            mem_free(tab_proc[i].kernel_stack,
                   tab_proc[i].kernel_stack_size);
            user_stack_free(tab_proc[i].user_stack,
                           tab_proc[i].user_stack_size);
            list_free(tab_proc[i].children);
            tab_proc[i].valide = 0;
        }
    }
}

int set_stdout(const char* name)
{
    fs_node_t *node;
    node = vfs_path_resolution(name);
    if(node == NULL) {
        return -1;
    }
    tab_proc[pid_cour-1].file_desc[1].node = node;
    return 0;
}
