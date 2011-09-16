#include "semaphore.h"
#include "queue.h"
#include "timer.h"
#include "list.h"
#include "sys.h"
#include "limits.h"


list_t sid_libres;

void sem_init()
{
    int i=0;
    sid_libres=list_new();
    /* semaphore descriptor array initialization */
    for(i = 0; i<NBSEM; i++){
        semaphores[i].valide=0;
        list_put_first(sid_libres, i);
    }

}

// /* Minimum free sid */
// static int sem_min_free = 0;
// 
// /* Find a sempahore number */
// static int sem_alloc(void)
// {
//     int return_me = sem_min_free;
//     int i;
//     for(i=sem_min_free+1; i<NBSEM; ++i) {
//         /* free SID find */
//         if(semaphores[i].valide==0) {
//             sem_min_free = i;
//             return return_me;
//         }
//     }
//     return -1;
// }
// 
// /* Free a semaphore number */
// static void sem_free(int sid)
// {
//     semaphores[sid].valide = 0;
//     /* sid is now free */
//     if (sem_min_free > sid) {
//         sem_min_free = sid;
//     }
// }

int screate(short int compteur)
{

    int i;
    /* invalid counter */
    if (compteur<0) return -1;

    /* searching free SID */
//     for(i=0; i<NBSEM; ++i) {
//         /* free SID find */
//         if(semaphores[i].valide!=1) {
//             /* semaphore initialization */
//             semaphores[i].c = compteur;
//             semaphores[i].proc_wait = list_new();
//             if(semaphores[i].proc_wait == 0) {
//                 return -3;
//             }
//             semaphores[i].valide = 1;
//             return i;
//         }
//     }
    if(list_is_empty(sid_libres)) return -1;

    i =list_get_first(sid_libres);
    assert(semaphores[i].valide!=1);

    semaphores[i].c = compteur;
    semaphores[i].proc_wait = list_new();

    if(semaphores[i].proc_wait == 0) {
        return -3;
    }
    semaphores[i].valide = 1;
    return i;

}

int wait(int sid){

    /* invalid SID */
    if(    sid < 0 
        || sid > NBSEM-1
        || semaphores[sid].valide != 1) {
        return -1;
    }

    /* capacity overflow */
    if (semaphores[sid].c==SHRT_MIN) return -2;

    /* decrease counter */
    semaphores[sid].c--;

    /* process must wait */
    if (semaphores[sid].c < 0){
        /* changing process state */
        tab_proc[pid_cour-1].state = STATE_WAIT_SEM;
        tab_proc[pid_cour-1].sid_wait = sid;

        /* adding process to the semaphore waiting process list */
        list_put_first(semaphores[sid].proc_wait, pid_cour);

        /* calling scheduler */
        scheduler();

        if(tab_proc[pid_cour-1].sid_reseted != 0) {
            tab_proc[pid_cour-1].sid_reseted = 0;
            return -4;
        }

        if(tab_proc[pid_cour-1].sid_deleted != 0) {
            return -3;
        }
    }

    return 0;
}

int try_wait(int sid){
    /* invalid SID */
    if(    sid < 0 
        || sid > NBSEM-1
        || semaphores[sid].valide!=1) {
        return -1;
    }

    /* process can decrease counter */
    if (semaphores[sid].c > 0){
        semaphores[sid].c--;
        return 0;
    }
    return -3;
}

int signal(int sid){
    int pid;

    /* invalid SID */
    if(sid < 0 || sid > NBSEM-1
        || semaphores[sid].valide!=1) {
        return -1;
    }

    /* capacity overflow */
    if (semaphores[sid].c==SHRT_MAX) {
        return -2;
    }

    /* increase counter */
    semaphores[sid].c++;

    /* there is a sleeped process */
    if (semaphores[sid].c <= 0) {
        /* awaking process */
        pid = list_get_last(semaphores[sid].proc_wait);
        if(pid >=0) {
            wakeup_process(pid);
        }
        scheduler();
    }

    return 0;
}

int signaln(int sid, short int count){
    /* invalid SID */
    if(sid < 0 || sid > NBSEM-1
        || semaphores[sid].valide!=1) {
        return -1;
    }

    /* invalid count */
    if (count < 0) {
        return -3;
    }

    /* capacity overflow */
    if (semaphores[sid].c > 0 && SHRT_MAX-semaphores[sid].c < count) {
        return -2;
    }

    int i, pid;

    /* processing count signal */
    for (i=0; i<count; i++){
        /* increase counter */
        semaphores[sid].c++;
        
        /* a process must be woken */
        if(semaphores[sid].c <= 0) {
            /* there is a slepped process */
            pid = list_get_last(semaphores[sid].proc_wait);
            if(pid >= 0) {
                /* awaking process */
                wakeup_process(pid);
            }
        }
    }
    scheduler();
    return 0;
}

int scount(int sid){
    int cpt;

    /* invalid SID */
    if(    sid < 0 
        || sid > NBSEM-1
        || semaphores[sid].valide!=1) {
        return -1;
    }

    /* getting counter */

    if(semaphores[sid].c < 0) {
        cpt = 0xFFFF;
        *((char*)&cpt) = semaphores[sid].c;
        return cpt;
    }
    else {
        return semaphores[sid].c;
    }
}

int sdelete(int sid){
    /* invalid SID */

    if(sid < 0 || sid > NBSEM-1
        || semaphores[sid].valide!=1) {
        return -1;
    }

    int pid;

    /* each waiting proc. must be woken and added in process delete list */
    while((pid = list_get_first(semaphores[sid].proc_wait)) >= 0) {

        if(tab_proc[pid-1].sid_reseted == 0) {
            tab_proc[pid-1].sid_deleted = 1;
        }

        /* awaking process */
        wakeup_process(pid);
    }

    list_free(semaphores[sid].proc_wait);

    semaphores[sid].valide = 0;
    
    list_put_first(sid_libres, sid);

    scheduler();

    return 0;
}

int sreset(int sid, short int count){
    
    /* invalid SID */
    if(    sid < 0 
        || sid > NBSEM-1
        || semaphores[sid].valide!=1
        || count<0   ) {
        return -1;
    }

    /* change counter */
    semaphores[sid].c = count;

    int pid;

    /* each waiting proc. must be woken and added in process reset list */
    while((pid=list_get_first(semaphores[sid].proc_wait)) >= 0) {

        tab_proc[pid-1].sid_reseted = 1;

        /* awaking process */
        wakeup_process(pid);
    }

    scheduler();

    return 0;
}

void sem_info(void)
{
    printf("SID  Counter  Waiting processes\n");
    int i;
    /* foreach SID in semaphore descriptor array */
    for(i=0;i<NBSEM;i++) {
        /* free SID */
        if(!semaphores[i].valide) continue;
        
        /* SID used */
        printf("%3d  %7d  ", i, semaphores[i].c);

        /* there is no process waiting */
        if(list_is_empty(semaphores[i].proc_wait)) {
            printf("<empty>\n");
        }

        /* foreach process waiting */
        list_iterator_t it = list_iterator(semaphores[i].proc_wait);
        while(list_iterator_has_next(it)) {
            int pid = list_iterator_next(it);
            /* display PID */
            printf("%d ", pid);
        }

        list_iterator_free(it);
        printf("\n");
    }
}
