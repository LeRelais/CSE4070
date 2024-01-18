#include <bitmap.h>
#include "vm/swap.h"
#include "vm/page.h"
#include "vm/frame.h"
#include "threads/vaddr.h"
#include "devices/block.h"
#include "userprog/syscall.h"

struct block *swap_block;
bool *swap_available;

void init_swap(){
    swap_block = block_get_role(BLOCK_SWAP);
    
    if(!swap_block)
        exit(-1);

    swap_available = (bool *)malloc(sizeof(bool) * (PGSIZE / 512));

    for(int i = 0; i < PGSIZE / 512; i++)
        swap_available[i] = true;

}

int swap_out(void *vaddr, void *paddr){
    int pos;
    for(int i = 0; i < PGSIZE / 512; i++){
        if(swap_available[i]){
            pos = i;
            break;
        }     
    }

    for(int i = 0; i < PGSIZE / 512; i++){
        block_write(swap_block, (PGSIZE / 512) * pos + i, 512 * i + paddr);
    }

    swap_available[pos] = false;

    return pos;
}

void swap_in(void *vaddr, void *paddr, int swap_idx){
    if(!swap_available[swap_idx]){
        for(int i = 0; i < PGSIZE / 512; i++){
            block_read(swap_available, swap_idx * (PGSIZE / 512) + i, 512 * i + paddr);
        }
        swap_available[swap_idx] = true;

        PAGE_ENTRY *tmp = find_page(&thread_current()->virtualMemoryTable, vaddr);
        tmp->swap = -1;
        pagedir_set_page(thread_current()->pagedir, pg_round_down(vaddr), pg_round_down(paddr), tmp->writable);
    }
}