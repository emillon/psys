#ifndef _LIST_H
#define _LIST_H 1

/*****************************
 *                           *
 *          list.h           *
 *   linked lists (of ints)  *  
 *                           *
 *****************************/

typedef struct _list *list_t;

/* Returns a fresh empty list */
list_t list_new(void);

/* Is the list empty ? */
int  list_is_empty(list_t l);

/* Empty the list */
void list_empty(list_t l);

/* Free memory allocated to l */
void list_free (list_t l);

/* Insert a new element */
void list_put_first  (list_t l, int element);

/* Get first element and remove it */
int  list_get_first  (list_t l);

/* Get last element (for FIFO use) */ 
int list_get_last (list_t l);

/* Is x in list l ? */
int  list_lookup (list_t l, int x);

/* Get a specific element and remove it. Return -1 on error */
int  list_delete  (list_t l, int e);

/* Iterator */
typedef struct _list_iterator * list_iterator_t;

/* New iterator */
list_iterator_t list_iterator(list_t l);

/* Are we done ? */
int list_iterator_has_next(list_iterator_t it);
/* Returns next element */
int list_iterator_next    (list_iterator_t it);

void list_iterator_free(list_iterator_t it);

#endif /* list.h */
