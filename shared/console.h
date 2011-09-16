#ifndef __CONSOLE_H__
#define __CONSOLE_H__

/*
 * This is the function called by printf to send its output to the screen. You
 * have to implement it in the kernel and in the user program.
 */
extern void console_putbytes(const char *s, int len);

struct cons_pal {
    int index;
    int r;      /* 0-63 */
    int g;      /* 0-63 */
    int b;      /* 0-63 */
};

/* Request numbers for cons_ioctl */
enum cons_ioctl_req {
    CONS_SETPAL,
    CONS_GETPAL,
    CONS_SETATTR,
    CONS_GETATTR,
    CONS_SETBUF,
    CONS_GETBUF
};

#ifdef __KERNEL__
#define SCR_PALETTE_MASK    0x3c6
#define SCR_PALETTE_READ    0x3c7
#define SCR_PALETTE_WRITE   0x3c8
#define SCR_PALETTE_DATA    0x3c9

extern void console_init(void);
extern void console_vfs_init(void);

#endif /* __KERNEL__ */
#endif /* console.h */
