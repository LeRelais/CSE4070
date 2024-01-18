#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <list.h>
#include <hash.h>
#include "lib/kernel/hash.h"
#include "threads/synch.h"
#include "threads/palloc.h"
#include "vm/page.h"

struct lock frame_lock;
struct list frame_list;
struct hash frame_hash;
struct list_elem *victim_pointer;

typedef struct _frame_entry
  {
    void *vaddr;               /* Kernel page, mapped to physical address */
    void *paddr;               /* User (Virtual Memory) Address, pointer to page */

    struct thread *thread;          

    struct hash_elem hElem;    
    struct list_elem lElem;    
  }FRAME_ENTRY;

void init_frame();

FRAME_ENTRY *allocate_frame(void *vaddr, void *paddr, bool writable, enum palloc_flags flags, uint32_t *pagedir);


bool free_frame(uint32_t *pagedir);
FRAME_ENTRY *move_victim_pointer();

#endif