#ifndef VM_SWAP_H
#define VM_SWAP_H

void init_swap();
int swap_out(void *vaddr, void *paddr);
void swap_in(void *vaddr, void *paddr, int swap_idx);


#endif