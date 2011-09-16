#ifndef _SEM_H_
#define _SEM_H_

#include "list.h"
#include "context.h"

#define NBSEM 10001

typedef struct _semaphore {
    short int c;                /* Counter */
    list_t proc_wait;           /* Waiting processes */
    int valide;                 /* Valid entry */
} semaphore;

semaphore semaphores[NBSEM];    /* array of semaphore descriptors */

void sem_init();

#endif
