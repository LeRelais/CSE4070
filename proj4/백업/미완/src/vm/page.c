#include <list.h>
#include <hash.h>
#include "vm/page.h"
#include "vm/frame.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "filesys/off_t.h"

static unsigned page_hash_value(const struct hash_elem *elem, void *aux){
    return hash_int(hash_entry(elem, PAGE_ENTRY, hElem));
}

static bool page_less(const struct hash_elem *hash1, const struct hash_elem *hash2, void *aux){
    return hash_entry(hash1, PAGE_ENTRY, hElem)->vaddr < hash_entry(hash2, PAGE_ENTRY, hElem)->vaddr;
}

static void page_destroy(struct hash_elem *elem, void *aux){
    free(hash_entry(elem, PAGE_ENTRY, hElem));
}

void *create_table(struct hash *table){
    hash_init(&table, page_hash_value, page_less, NULL);
}

void destroy_table(struct hash *table){
    hash_destroy(table, page_destroy);
}

bool page_set_swap(PAGE_TABLE *pt, void *page, size_t swap){
    PAGE_ENTRY *target = find_page(pt, page);

    if(target){
        target->status = SWAP;
        target->paddr = NULL;
        target->swap = swap;
        return true;
    }

    return false;
}

bool page_set_dirty(PAGE_TABLE *pt, void *page, bool dirty){
    PAGE_ENTRY *target = find_page(pt, page);

    if(!target)
        exit(-1);

    target->dirty = dirty;
    
    
}


PAGE_ENTRY *find_page(PAGE_TABLE *pt, void *page){
    PAGE_ENTRY tmp;
    tmp.vaddr = pg_round_down(page);
    
    if(hash_find(&pt->page_table, &tmp.hElem)){
        return hash_entry(hash_find(&pt->page_table, &tmp.hElem), PAGE_ENTRY, hElem);
    }
    
    return NULL;
}