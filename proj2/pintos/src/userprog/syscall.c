#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "filesys/inode.h"
#include "filesys/off_t.h"
#include "threads/synch.h"

static void syscall_handler (struct intr_frame *);
struct lock sync;

bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
void close (int fd);

struct file 
  {
    struct inode *inode;        /* File's inode. */
    off_t pos;                  /* Current position. */
    bool deny_write;            /* Has file_deny_write() been called? */
  };

syscall_init (void) 
{
  lock_init(&sync);
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
    case SYS_CREATE:
      if(is_user_vaddr(f->esp+4) && is_user_vaddr(f->esp+8)){
        f->eax = create((const char*)*(uint32_t*)(f->esp+4), (unsigned)*(uint32_t*)(f->esp+8));
      }
      else
        exit(-1);
      break;
    case SYS_REMOVE:
      if(is_user_vaddr(f->esp+4))
        f->eax = remove((const char*)*(uint32_t*)(f->esp+4));
      else
        exit(-1);
      break;
    case SYS_OPEN:
      if(is_user_vaddr(f->esp+4))
        f->eax = open((const char*)*(uint32_t*)(f->esp+4));
      else
        exit(-1);
      break;
    case SYS_FILESIZE:
      if(is_user_vaddr(f->esp+4))
        f->eax = filesize((int)*(uint32_t*)(f->esp+4));
      else
        exit(-1);
      break;
    case SYS_READ: 
      if(is_user_vaddr(f->esp+4) &&is_user_vaddr(f->esp+8) && is_user_vaddr(f->esp+12))
        f->eax = read((int)*(uint32_t*)(f->esp+4), (void*)*(uint32_t*)(f->esp+8), (unsigned)*(uint32_t*)(f->esp+12));
      else
        exit(-1);
      break;
    case SYS_WRITE:
      if(is_user_vaddr(f->esp+4) &&is_user_vaddr(f->esp+8) && is_user_vaddr(f->esp+12))
        f->eax = write((int)*(uint32_t*)(f->esp+4), (const void*)*(uint32_t*)(f->esp+8), (unsigned)*(uint32_t*)(f->esp+12));
      else
        exit(-1);
      break;
    case SYS_SEEK:
      if(is_user_vaddr(f->esp+4)){
        seek((int)*(uint32_t*)(f->esp+4), (unsigned)*(uint32_t*)(f->esp+8));
      }
      else{
        exit(-1);
      }
      break;
    case SYS_TELL:
      if(is_user_vaddr(f->esp+4))
        f->eax = tell((int)*(uint32_t*)(f->esp+4));
      else
        exit(-1);
      break;
    case SYS_CLOSE:
      if(is_user_vaddr(f->esp+4))
        close((int)*(uint32_t*)(f->esp+4));
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
      if(is_user_vaddr(f->esp+4) && is_user_vaddr(f->esp+8) && is_user_vaddr(f->esp+12) && is_user_vaddr(f->esp+16))
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

  struct list_elem *listElem = list_begin(&thread_current()->child);
  struct thread *tmpThread;

  for(int i = 3; i < 128; i++){
    if(thread_current()->fd[i])
      close(i);
  }
  while(1){
    if(listElem == list_end(&thread_current()->child))
      break;
    
    tmpThread = list_entry(listElem, struct thread, childElem);
    
    if(tmpThread->waitStatus){
      process_wait(tmpThread->tid);
    }

    listElem = list_next(listElem);
  }
  file_close(thread_current()->runningFile);
  thread_exit();
}

pid_t exec (const char *file){
  if(!file)
    return -1;

  return process_execute(file);
}

int wait (pid_t pid){
  return process_wait((tid_t)pid);
}

bool create (const char *file, unsigned initial_size){  
  if(!file){
    exit(-1);
  }

  return filesys_create(file, initial_size);
}

bool remove (const char *file){
  if(!file)
    exit(-1);

  return filesys_remove(file);
}

int open (const char *file)
{
  if(!file)
    exit(-1);

  //lock_acquire(&sync);
  //struct file *tmp = filesys_open(file);
  //if(tmp){
  //  for(int i = 3; i < 128; i++){
  //    if(!thread_current()->fd[i]){
  //      if(strcmp(thread_current()->name, file) == 0)
  //        file_deny_write(tmp);
  //      thread_current()->fd[i] = tmp;
  //      int res = i;
  //      lock_release(&sync);
  //      return res;
  //    }
  //  }
  //}
  //else{
  //  lock_release(&sync);
  //  return -1;
  //}
  //lock_release(&sync);
  //return -1;

  int ret = -1, i;
  struct thread *tmp  = thread_current();
  lock_acquire(&sync);
  struct file *fp = filesys_open(file);

  if(fp == NULL)
    ret = -1;
  else{
    for(i = 3; i < 128; i++){
      if(thread_current()->fd[i] == NULL){
 
        thread_current()->fd[i] = fp;
        ret = i;
        break;
      }
    }
  }
  lock_release(&sync);
  return ret;
}

int filesize (int fd) 
{
  if(!thread_current()->fd[fd])
    exit(-1);

  return file_length(thread_current()->fd[fd]);
}


int read (int fd, void *buffer, unsigned length){
  if(!is_user_vaddr(buffer))
    exit(-1);

  int res;
  if(fd == 0){
      /*input.c   use input_getc(void) */
      for(unsigned int i = 0; i < length; i++){
        if((char*)buffer == '\0'){
          return i;
        }
      }
  }
  else if(fd > 2){
        if(thread_current()->fd[fd]){
          lock_acquire(&sync);
          res = file_read(thread_current()->fd[fd], buffer, length);
          lock_release(&sync);
          return res;
        }
        else{
          exit(-1);
        }
  }
  
  return -1;

  //int i, ret;
  //lock_acquire(&sync);
  //if(fd == 0){
  //  for(i = 0; i < length; i++){
  //    if(((char*)buffer)[i] == '\0')
  //      break;
  //  }
  //  ret = i;
  //}
  //else if(fd > 2){
  //  if(thread_current()->fd[fd] == NULL){
  //    lock_release(&sync);
  //    return -1;
  //  }
  //  ret = file_read(thread_current()->fd[fd], buffer, length);
  //}
  //lock_release(&sync);
  //return ret;
}

int write (int fd, const void *buffer, unsigned length){;
  //if(!is_user_vaddr(buffer))
  //  exit(-1);
  //
  //lock_acquire(&sync);
  //if(fd == 1){
  //    /*console.c   use putbuf()*/
  //    putbuf(buffer, length);
  //    lock_release(&sync);
  //    return length;
  //}
  //else if(fd > 2){
  //    if(!thread_current()->fd[fd]){
  //      lock_release(&sync);
  //      exit(-1);
  //    }
  //    if(thread_current()->fd[fd]->deny_write)
  //      file_deny_write(thread_current()->fd[fd]);
  //    int res = file_write(thread_current()->fd[fd], buffer, length);
  //    lock_release(&sync);
  //    return res;
  //}
  //lock_release(&sync);
  //return -1;

  int ret = -1;
  struct thread *tmp = thread_current();
  lock_acquire(&sync);
  if(fd == 1){
    putbuf(buffer, length);
    ret = length;
  }
  else if(fd > 2){
    if(tmp->fd[fd] == NULL){
      lock_release(&sync);
      exit(-1);
    }
    //if(thread_current()->fd[fd]->deny_write){
    //  file_deny_write(thread_current()->fd[fd]);
    //}
    ret = file_write(tmp->fd[fd], buffer, length);
  }
  lock_release(&sync);
  return ret;
}

void seek (int fd, unsigned position) 
{ 
  if(fd < 2)
    return;

  if(!thread_current()->fd[fd])
    return;

  file_seek(thread_current()->fd[fd], position);
}

unsigned tell (int fd) 
{
  if(!thread_current()->fd[fd])
    exit(-1);
  return file_tell(thread_current()->fd[fd]);
}

void close (int fd){
  if(thread_current()->fd[fd] == NULL)
    exit(-1);
  thread_current()->fd[fd] = NULL;
  file_close(thread_current()->fd[fd]);
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
  }
  return res;
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