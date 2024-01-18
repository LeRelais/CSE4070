#include <list.h>
#include <hash.h>
#include "vm/page.h"
#include "vm/frame.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "filesys/off_t.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"

static unsigned frame_hash_value(const struct hash_elem *hElem, void *aux){
    return hash_bytes(&(hash_entry(hElem, FRAME_ENTRY, hElem))->paddr, sizeof(hash_entry(hElem, FRAME_ENTRY, hElem)->paddr));
}

static bool frame_less(const struct hash_elem *frame1, const struct hash_elem *frame2, void *aux){
    return hash_entry(frame1, FRAME_ENTRY, hElem)->paddr < hash_entry(frame2, FRAME_ENTRY, hElem)->paddr;
}

void init_frame(){
    list_init(&frame_list);
    lock_init(&frame_lock);
    victim_pointer = NULL;
}

FRAME_ENTRY *allocate_frame(void *vaddr, void *paddr, bool writable, enum palloc_flags flags, uint32_t *pagedir){
    lock_acquire(&frame_lock);

    FRAME_ENTRY *tmp = (FRAME_ENTRY *)malloc(sizeof(FRAME_ENTRY));
    tmp->vaddr = pg_round_down(vaddr);
    tmp->thread = thread_current();

    if(!paddr){
        paddr = palloc_get_page(flags);

        if(!paddr){
            free_frame(pagedir);
            paddr = palloc_get_page(flags);
        }

        tmp->paddr = pg_round_down(paddr);
    }

    tmp->paddr = pg_round_down(paddr);
    list_push_back(&frame_list, &tmp->lElem);

    lock_release(&frame_lock);
}

bool free_frame(uint32_t *pagedir){
    if(!list_empty(&frame_list)){
        FRAME_ENTRY *first_choice = move_victim_pointer();
        FRAME_ENTRY *tmp = NULL;
        if(pagedir_is_accessed(pagedir, first_choice->vaddr)){
                pagedir_set_accessed(pagedir, first_choice->vaddr, false);
            }
        while(1){
            tmp = move_victim_pointer();

            if(tmp == first_choice)
                break;

            if(pagedir_is_accessed(pagedir, tmp->vaddr)){
                pagedir_set_accessed(pagedir, tmp->vaddr, false);
            }
            else
                break;
        }

        PAGE_ENTRY *evict = find_page(&tmp->thread->virtualMemoryTable, tmp->vaddr);
        evict->paddr = NULL;
        pagedir_clear_page(tmp->thread->pagedir, tmp->vaddr);
        palloc_free_page(tmp->paddr);
        list_remove(&tmp->lElem);
        free(tmp);
        return true;
    }
    exit(-1);
}

FRAME_ENTRY *move_victim_pointer(){
    if(!list_begin(&frame_list) && !list_end(&frame_list) && list_empty(&frame_list))
        exit(-1);

    if(!victim_pointer){
        victim_pointer = list_begin(&frame_list);
    }

    FRAME_ENTRY *tmp = list_entry(victim_pointer, FRAME_ENTRY, lElem);
    
    if(victim_pointer != list_end(&frame_list))
        victim_pointer = list_next(&victim_pointer);
    else   
        victim_pointer = list_begin(&frame_list);
    
    return tmp;
}