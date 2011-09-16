#include "sys.h"
#include "timer.h"
#include "mem.h"
#include "user_stack_mem.h"
#include "syscall_handlers.h"

void exit(int retval) {
    int ppid, pid_child;
    list_iterator_t it;

    /* wake up father if waiting */
    ppid = tab_proc[pid_cour-1].pid_parent;

    /* father is not Init */
    if(ppid > 1) {
        /* father is waiting current process or first process */
        if(tab_proc[ppid-1].state==STATE_WAIT_CHILD &&
		   (pid_cour==tab_proc[ppid-1].pid_wait 
			|| tab_proc[ppid-1].pid_wait==-1)) {
            tab_proc[ppid-1].pid_wait = pid_cour;
            wakeup_process(ppid);
        }
    }

	/* kill children if zombies or attach process to Init if alive */	   
    it = list_iterator(tab_proc[pid_cour-1].children);

    while(list_iterator_has_next(it)) {
        pid_child = list_iterator_next(it);

        if(tab_proc[pid_child-1].state == STATE_ZOMBIE) {
        	/* killing zombie child */
            tab_proc[pid_child-1].state = STATE_DEAD;
        }
        else {
        	/* attaching alive child to Init */
            tab_proc[pid_child-1].pid_parent = 1;
            list_put_first (tab_proc[0].children, pid_child);
        }
    }

    list_iterator_free(it);

    /* change current process state */
    if(ppid>1) {
		/* if father is not Init, current process become zombie */
        tab_proc[pid_cour-1].state = STATE_ZOMBIE;
        tab_proc[pid_cour-1].val_ret = retval;
    }
    else {
        /* if father is Init, the current process is killed */
        tab_proc[pid_cour-1].state = STATE_DEAD;
        list_delete(tab_proc[0].children, pid_cour);
    }
   
    /* calling scheduler */
    scheduler();

    while(1);
}

int getprio(int pid) {
     /* invalid PID */
    if(    pid==0
        || pid > NBPROC
        || tab_proc[pid-1].valide==0
        || tab_proc[pid-1].state==STATE_DEAD
        || tab_proc[pid-1].state==STATE_ZOMBIE) {
        return -1;
    }

    return tab_proc[pid-1].priority;
}

int chprio(int pid, int newprio)
{
    int oldprio = tab_proc[pid-1].priority;
    context * elem;

    /* invalid PID */
    if (   pid <= 0
        || pid > NBPROC
        || tab_proc[pid-1].valide == 0
        || tab_proc[pid-1].state == STATE_ZOMBIE
        || tab_proc[pid-1].state == STATE_DEAD
        ) {
        return -1;
    }

    /* bad priority */
    if(newprio <= 0 || newprio > MAXPRIO) {
        return -2;
    }

    /* priority doesn't change */
    if(newprio == tab_proc[pid-1].priority) {
        /* calling scheduler */
        scheduler();
        return oldprio;
    }

    /* change priority */
    tab_proc[pid-1].priority = newprio;

    /* process is activable */
    if(tab_proc[pid-1].state==STATE_ACTIVABLE) {
        /* searching process in activable process list */
        queue_for_each(elem, &proc_queue_activ, context, activable) {
            /* process find */
            if(elem->pid == pid) {
                /* remove then reinsert process in the list */
                queue_del(elem, activable);
                queue_add(elem, &proc_queue_activ, context, activable,
                                                            priority);
                /* calling scheduler */
                scheduler();
                return oldprio;
            }
        }
    }

    /* process is not activable */
    /* calling scheduler */
    scheduler();
    return oldprio;
}

int getpid() {
    return pid_cour;
}

int getppid() {
    return tab_proc[pid_cour-1].pid_parent;
}

int waitpid(int pid, int *retvalp) {
    retvalp = check_pointer(retvalp, SYSCALL_WAITPID);
    int pid_child, trouve = 0; 
    list_iterator_t it;
    
    /* process can't wait itself */
    if(pid == pid_cour) {
        return -1;
    }

    /* waiting first child */
    if(pid < 0) {
        
        /* there is no child */
        if(list_is_empty(tab_proc[pid_cour-1].children)) {
            return -1;
        }

        /* searching a zombie child */
        it = list_iterator(tab_proc[pid_cour-1].children);

        while(list_iterator_has_next(it)) {
            pid_child = list_iterator_next(it);

            /* zombie child find */
            if(tab_proc[pid_child-1].state == STATE_ZOMBIE) {
                trouve = 1;
                break;
            }
        }

        list_iterator_free(it);
        
        /* no zombie find */
        if(trouve==0) {
            /* current process must wait */
            tab_proc[pid_cour-1].state = STATE_WAIT_CHILD;
            tab_proc[pid_cour-1].pid_wait = -1;
            /* calling scheduler */
            scheduler();

            /* searching zombie child */
            /*it = list_iterator(tab_proc[pid_cour-1].children);

            while(list_iterator_has_next(it)) {
                pid_child = list_iterator_next(it);
    */
                /* zombie find */
      /*          if(tab_proc[pid_child-1].state == STATE_ZOMBIE) {
                    break;
                }
            }*/

            pid_child = tab_proc[pid_cour-1].pid_wait;
        }

        /* getting return value */
        if (retvalp != (void*)0) {
            if(tab_proc[pid_child-1].killed==1) {
                *retvalp = 0;
            }
            else {
                *retvalp = tab_proc[pid_child-1].val_ret;
            }
        }

        /* killing child waited */
        tab_proc[pid_child-1].state = STATE_DEAD;
        list_delete(tab_proc[pid_cour-1].children, pid_child);

        return pid_child;
    }
    /* waiting child with pid */
    else {
        /* invalid PID */
        if(pid==0) {
            return -2;
        }

        /* searching PID to be waited */
        it = list_iterator(tab_proc[pid_cour-1].children);

        while(list_iterator_has_next(it)) {
            pid_child = list_iterator_next(it);
            /* PID find */
            if(pid_child==pid) {
                trouve = 1;
                break;
            }
        }

        list_iterator_free(it);

        /* PID not find so invalid */
        if(trouve == 0) {
            return -3;
        }

        /* PID child is not a zombie */
        if(tab_proc[pid-1].state != STATE_ZOMBIE) {
            /* current process must wait */
            tab_proc[pid_cour-1].state = STATE_WAIT_CHILD;
            tab_proc[pid_cour-1].pid_wait = pid;
            /* calling scheduler */
            scheduler();
        }

        /* getting return value */
        if(retvalp != (void*)0) {
            if(tab_proc[pid-1].killed==1) {
                *retvalp = 0;
            }
            else {
                *retvalp = tab_proc[pid-1].val_ret;
            }
        }

        /* killing child waited */
        tab_proc[pid-1].state = STATE_DEAD;
        list_delete(tab_proc[pid_cour-1].children, pid);

        return pid;
    }
}


int kill(int pid) {
    int ppid, pid_child;
    list_iterator_t it;

    /* invalid PID */
    if(   pid <=1 
       || pid>NBPROC 
       || tab_proc[pid-1].valide == 0
       || tab_proc[pid-1].state == STATE_DEAD 
       || tab_proc[pid-1].state == STATE_ZOMBIE) {
        return -1;
    }

    /* notify semaphore if process is waited after wait() */
    if(tab_proc[pid-1].state == STATE_WAIT_SEM) {
        semaphores[tab_proc[pid-1].sid_wait].c++;
    }

    /* wake up father if waiting */
    ppid = tab_proc[pid-1].pid_parent;

    /* father is not Init */
    if(ppid > 1) {
        /* father is waiting current process or first process */
        if(tab_proc[ppid-1].state == STATE_WAIT_CHILD &&
(pid==tab_proc[ppid-1].pid_wait || tab_proc[ppid-1].pid_wait==-1)) {            
            wakeup_process(ppid);
        }
    }

    /* kill children if zombies or attach process to Init if alive */ 
    it = list_iterator(tab_proc[pid-1].children);

    while(list_iterator_has_next(it)) {
        pid_child = list_iterator_next(it);

        if(tab_proc[pid_child-1].state == STATE_ZOMBIE) {
            /* killing zombie child */
            tab_proc[pid_child-1].state = STATE_DEAD;
        }
        else {
            /* attaching alive child to Init */
            tab_proc[pid_child-1].pid_parent = 1;
            list_put_first(tab_proc[0].children, pid_child);
        }
    }

    list_iterator_free(it);

    /* remove process from activable process list */
    if(tab_proc[pid-1].state==STATE_ACTIVABLE) {
        queue_del(&tab_proc[pid-1], activable);
    }

    /* changing process state */
    if(ppid>1) {
        /* if father is not Init, current process become zombie */
        tab_proc[pid-1].state = STATE_ZOMBIE;
        tab_proc[pid-1].killed = 1;
    }
    else {
        /* if father is Init, current process is killed */
        tab_proc[pid-1].state = STATE_DEAD;
        list_delete(tab_proc[0].children, pid);
    }
    scheduler();
    return 0;
}

int start(int (*ptfunc)(void *), unsigned long ssize,
                                int prio, const char * name, void * arg)
{
    int ret;
    ptfunc = check_pointer(ptfunc, SYSCALL_START);
    /* stack size too high */
    if(ssize > 0xffffffe7) {
        return -1;
    }
    /* increase stack size for argument, program and signal return adress */
    ssize += 3*sizeof(unsigned long);
    ret =  context_create_user((void *) ptfunc, ssize, prio, name, arg);
    if(ret > 0) {
        scheduler();
    }
    return ret;
}

/* Safety checks */

static inline int is_pointer_userspace(const void* p)
{
    return (unsigned long) p >= (unsigned long) 0x1000000;
}

void* check_pointer(const void* p, int code)
{
    if (is_in_syscall(code) && !is_pointer_userspace(p)) {
        return (void*) 0;
    }
    return (void*) p;
}
