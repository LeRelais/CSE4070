#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  //printf("\n\n %d \n\n", *(uint32_t *)(f->esp));
  //hex_dump((uintptr_t)f->esp,f->esp,100,true);
  switch(*(uint32_t *)(f->esp)){
    case SYS_HALT:
      shutdown_power_off();
      break;
    case SYS_EXIT:
      if(is_user_vaddr(f->esp+4))
        exit((int)*(uint32_t*)(f->esp+4));
      else 
        exit(-1);
      break;
    case SYS_EXEC:
      if(is_user_vaddr(f->esp+4))
        f->eax = exec((const char*)*(uint32_t *)(f->esp+4));
      else
        exit(-1);
      break;
    case SYS_WAIT:
      if(is_user_vaddr(f->esp+4))
        f->eax = wait((pid_t)*(uint32_t *)(f->esp+4));
      else
        exit(-1);
      break;
    case SYS_READ: 
      if(is_user_vaddr(f->esp+4))
        f->eax = read((int)*(uint32_t*)(f->esp+4), (void*)*(uint32_t*)(f->esp+8), (unsigned)*(uint32_t*)(f->esp+12));
      else
        exit(-1);
      break;
    case SYS_WRITE:
      if(is_user_vaddr(f->esp+4))
        f->eax = write((int)*(uint32_t*)(f->esp+4), (const void*)*(uint32_t*)(f->esp+8), (unsigned)*(uint32_t*)(f->esp+12));
      else
        exit(-1);
      break;
    case SYS_FIBO:
      if(is_user_vaddr(f->esp+4))
        f->eax = fibo((int)*(uint32_t*)(f->esp+4));
      else
        exit(-1);
      break;
    case SYS_MAX_FOUR:
      if(is_user_vaddr(f->esp+4))
        f->eax = maxOfFour((int)*(uint32_t*)(f->esp+4), (int)*(uint32_t*)(f->esp+8), (int)*(uint32_t*)(f->esp+12), (int)*(uint32_t*)(f->esp+16));
      else
        exit(-1);
      break;
  }
 //printf ("system call!\n");
 //
 //thread_exit ();
}

void exit (int status){
  printf("%s: exit(%d)\n", thread_current()->name, status);
  thread_current()->ExitStatus = status;
  thread_exit();
}

pid_t exec (const char *file){
  return process_execute(file);
}

int wait (pid_t pid){
  return process_wait((tid_t)pid);
}

int read (int fd, void *buffer, unsigned length){
  if(!fd){
      /*input.c   use input_getc(void) */
      for(unsigned int i = 0; i < length; i++){
        if((char*)buffer == '\0')
          return i;
      }
  }
  else{
    return -1;
  }
}

int write (int fd, const void *buffer, unsigned length){
  if(fd){
      /*console.c   use putbuf()*/
      putbuf(buffer, length);
      return length;
  }
  else
    return -1;
}

int fibo(int n){
  int res = 0;
  if(n == 1)
    return 1;
  else if(n == 2){
    return 1;
  }
  else{
     int x = 1, y;
    for(int i = 3; i <= n; i++){
      if(i == 3){
        res = 2;
        y = x;
        x = res;
      }
      else{
        res += y;
        y = x;
        x = res;
      }
    }
    return res;
  }
}

int maxOfFour(int a, int b, int c, int d){
   int res;
   if(a > b)
     res = a;
   else 
     res = b;

   if(res < c)
     res = c;

   if(res < d)
     res = d;

   return res;

}