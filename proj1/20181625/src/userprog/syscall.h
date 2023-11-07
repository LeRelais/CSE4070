#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

typedef int pid_t;

void syscall_init (void);
void exit(int status);
pid_t exec(const char *file);
int read (int fd, void *buffer, unsigned length);
int write (int fd, const void *buffer, unsigned length);
int fibo(int n);
int maxOfFour(int a, int b, int c, int d);
#endif /* userprog/syscall.h */
