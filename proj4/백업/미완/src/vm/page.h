#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include <list.h>
#include <threads/thread.h>
#include "filesys/off_t.h"

typedef enum _PAGE_STATUS{
    BINARY,
    FILE,
    SWAP
}PAGE_STATUS;

typedef struct _page_table {
    struct hash page_table;
}PAGE_TABLE;

typedef struct _page_entry {
    PAGE_STATUS status;

    void *vaddr;     /* Virtual address of the page. */
    void *paddr;     /* Physical address of page */

    bool dirty;
    
    size_t swap;  

    struct file *f;
    off_t offset;
    uint32_t read_bytes, zero_bytes;
    bool writable;

    struct hash_elem hElem;
}PAGE_ENTRY;

void *create_table(struct hash *table);
void destroy_table(struct hash *table);

bool page_insert(void *vaddr, void *paddr, bool writable);
bool has_entry(PAGE_TABLE *pt, void *page);
PAGE_ENTRY *find_page(PAGE_TABLE *pt, void *page);
#endif