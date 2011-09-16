#ifndef _SYSCALLS_H
#define _SYSCALLS_H 1
#include "types.h"
#include "syscall_numbers.h"
#include "file_structs.h"
#define STACK_SIZE 1000000

/*******************************
 *          Processes          *
 *******************************/
int start(int (*ptfunc)(void *), unsigned long ssize,
                                    int prio, const char * name, void * arg);

void exit(int retval);

int getprio(int pid);
int chprio (int pid, int newprio);

int getpid();
int getppid();

int waitpid(int pid, int *retvalp);
int kill(int pid);

void context_ps(void);

/*******************************
 *          Semaphores         *
 *******************************/

int screate(short int count);
int scount (int sem);
int sdelete(int sem);
int sreset (int sem, short int count);

int wait(int sem);
int try_wait(int sem);

int signal (int sem);
int signaln(int sem, short int count);

void sem_info(void);

/******************************
 *          Console           *
 ******************************/

void            cons_echo (int on);
void            cons_write(const char*s, int len);
unsigned long   cons_read (char* buf, unsigned long len);
void            cons_ioctl(int request, void* p);
void            clear(void);

/******************************
 *          Timing            *
 ******************************/
unsigned long current_clock(void);
void clock_settings(unsigned long * quartz, unsigned long * ticks);

void wait_clock(unsigned long clock);
void usleep(unsigned long microseconds);

/******************************
 *            VFS             *
 ******************************/

/* Open a file. Return a file descriptor, or -1 on error */
int open    (const char *name);

/* Close a file descriptor. Return 0 on success, <0 on error */
int close   (int fd);

/*
 * Read len bytes from fd to buf.
 * Return bytes effectively read, -1 on error, 0 at end of file.
 */
int read    (int fd, char* buf, unsigned long len);

/*
 * Write len bytes from buf to fd.
 * Return bytes effectively written, -1 on error.
 */
int write (int fd, const char* buf, unsigned long len);

/*
 * Fill dirp with nth entry from directory opened at fd
 * Return value :
 * -1 on error
 *  0 at directory end
 *  1 if ok
 */
int readdir (int fd, struct dirent *dirp, int n);

/* Get information from a file */
int fstat   (int fd, struct stat *s);


/*Signals*/
void signal_send(int pid, int signalid);
void sigaction(int signalid, void* ad_trt);


/* Get current working directory */
char* getcwd (char* buf, size_t len);

/* Change working directory */
int   chdir  (const char *path);

/* Change standard output */
int set_stdout(const char* filename);

/******************************
 *           Misc             *
 ******************************/

void reboot(void);
void buzzer_beep(int note, int milliseconds);

#endif /* syscalls.h */
