#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

typedef int pid_t;

void syscall_init (void);
void exit(int status);
pid_t exec(const char *file);
int wait (pid_t);
//bool create (const char *file, unsigned initial_size);
//bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned length);
int write (int fd, const void *buffer, unsigned length);
void seek (int fd, unsigned position);
unsigned tell (int fd);
int fibo(int n);
int maxOfFour(int a, int b, int c, int d);
#endif /* userprog/syscall.h */
