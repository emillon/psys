#include "list.h"
#include "mem.h"
#include "debug.h"
#define NULL ((void*) 0)

struct _list {
    int element;
    struct _list *next;
};

static list_t list_alloc(void)
{
    void * l;
    l = mem_alloc (sizeof(struct _list));
    return (list_t) l;
}

static void list_free_block (list_t block)
{
    mem_free (block, sizeof(struct _list));
}

list_t list_new(void)
{
    list_t l;
    l = list_alloc();
    //assert(l!=NULL);
    if(l==NULL)
        return NULL;
    l->element = -1;
    l->next    = NULL;
    return l;
}

int list_is_empty(list_t l)
{
    assert(l != NULL);
    return (l->next == NULL);
}

void list_empty(list_t l)
{
    while(!list_is_empty(l))
    {
        list_get_first(l);
    }
}

void list_free (list_t l)
{
    list_t to_free;
    while(l != NULL){
        to_free=l;
        l=l->next;
        list_free_block(to_free);
    }
}

void list_put_first (list_t l, int element)
{
    assert(l != NULL);
    assert(element >= 0);
    list_t block = list_alloc();
    block->element = element;
    block->next = l->next;
    l->next = block;
}

int list_get_first (list_t l)
{
    assert(l != NULL);
    list_t to_free;
    if(!list_is_empty(l)) {
        int to_return;
        to_return = l->next->element;
        to_free = l->next;
        l->next = l->next->next;
        list_free_block(to_free);
        return to_return;
    } else {
        return -1;
    }
}

int list_get_last (list_t l)
{
    assert(l!=NULL);
    if(l->next==NULL)
        return -1;
    while(l->next->next!=NULL){
        l=l->next;
    }
    /* OK, here l->next->next==NULL */
    list_t free_me = l->next;
    int return_me = l->next->element;
    l->next=NULL;
    list_free_block(free_me);
    return(return_me);
}

int list_lookup (list_t l, int x)
{
    assert(l!=NULL);
    l=l->next;
    for(;l!=NULL;l=l->next){
        if(l->element == x)
            return 1;
    }
    return 0;
}

int list_delete (list_t l, int e)
{
    assert(l!=NULL);
    if(l->next==NULL)
        return -1;
    while(l->next->element!=e){
        l=l->next;
        if(l->next==NULL)
            return -1;
    }
    /* Here l->next->element==e */
    list_t free_me = l->next;
    l->next=l->next->next;
    list_free_block(free_me);
    return e;
}

/* Iterator */

struct _list_iterator {
    struct _list *cur_pos;
};

list_iterator_t list_iterator(list_t l)
{
    list_iterator_t it;
    assert(l!=NULL);
    it = mem_alloc(sizeof(struct _list_iterator));
    it->cur_pos = l;
    return it;
}

int list_iterator_has_next(list_iterator_t it)
{
    return (it->cur_pos->next!=NULL);
}

int list_iterator_next (list_iterator_t it)
{
    it->cur_pos = it->cur_pos->next;
    return it->cur_pos->element;
}

void list_iterator_free(list_iterator_t it)
{
    mem_free(it, sizeof(struct _list_iterator));
}
