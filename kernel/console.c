#include "cpu.h"
#include "processor_structs.h"
#include "console.h"
#include "context.h"
#include "kbd.h"
#include "irq.h"

#include "string.h"
#include "stdlib.h"
#include "mem.h"
#include "sys.h"
#include "syscall_handlers.h"

#define NB_COL 80
#define NB_ROW 25

/* MC6845 registers */
#define SCREEN_SEL  0x03d4
#define SCREEN_REG  0x03d5

/* Keyboard registers */
#define KB_DATA             0x060
#define KB_STATUS           0x064

extern void int33(void);

/* Main video memory */
static struct console_character * const video_mem =
                                        (struct console_character*) 0xB8000;

struct console_character {
    char c;             /* Character itself */
    char attr;
    /*
    unsigned int  blink : 1;      Blinking ?       
    unsigned int  bg    : 3;      Background color 
    unsigned int  fg    : 4;      Foreground color 
    */
} __attribute__((packed));


#define INPUT_BUFFER_SIZE 1024
struct ringbuffer {
    char  in_buf[INPUT_BUFFER_SIZE];
    char* in_start;
    unsigned long in_size;
};

/* Virtual consoles */
struct virtual_console {
    struct console_character* mem;       // main array
    int pos;                            /* Cursor position */
    int echo;
    int buffered;
    int current_attr;
    struct ringbuffer buf;
    int waiting_pid;
};

#define VIRTUAL_CONSOLES 4
static struct virtual_console virtual_consoles[VIRTUAL_CONSOLES];
static struct virtual_console *vc_current;
static fs_node_t vc_fs_node[VIRTUAL_CONSOLES];

static void console_update_cursor(struct virtual_console *vc);
static void vc_draw (struct virtual_console *vc)
{
    memcpy(video_mem, vc->mem, NB_COL*NB_ROW*sizeof(struct console_character));
    console_update_cursor(vc);
}

static void ringbuffer_init(struct ringbuffer *r);
static void vc_init(struct virtual_console *vc)
{
    ringbuffer_init(&vc->buf);
    vc->pos          = 0;
    vc->echo         = 1;
    vc->buffered     = 1;
    vc->current_attr = 0x07;
    vc->waiting_pid  = -1;
}

static void vc_flip(int vc_number) {
    struct virtual_console *which;
    if(vc_number >= 0 && vc_number < VIRTUAL_CONSOLES) {
        which = &virtual_consoles[vc_number];
        vc_draw(which);
        video_mem [NB_COL*NB_ROW-1].c = '0'+vc_number;
        vc_current = which;
    }
}

/******************************** 
 ****** Ringbuffer control ****** 
 ********************************/

static void ringbuffer_init(struct ringbuffer *r)
{
	r->in_start = r->in_buf;
	r->in_size = 0;
}

static void buffer_push(struct virtual_console *vc, char key)
{
    if(vc->buffered) {
        if(key == '\n') {
            if(vc->waiting_pid!=-1) {
                signal_new_io(vc->waiting_pid);
            }
        }
    } else {
        signal_new_io(vc->waiting_pid);
    }
 	if (vc->buf.in_size == INPUT_BUFFER_SIZE) /* buffer full */
		return;
	if (vc->buf.in_start + vc->buf.in_size < vc->buf.in_buf+INPUT_BUFFER_SIZE) {
		/* occupation : |    XXXXXXXX      | */
		vc->buf.in_start[vc->buf.in_size++] = key;
	} else {
		/* occupation : |XXXX            XX| */
		vc->buf.in_start[vc->buf.in_size++ - INPUT_BUFFER_SIZE] = key;
	}
}

static char buffer_pop(struct virtual_console *vc)
{
	char res;
	if(vc->buf.in_size == 0) {/* empty buffer */
		if(vc->buf.in_size==0) {
            vc->waiting_pid = getpid();
            /* put process to sleep */
            tab_proc[getpid()-1].state=STATE_WAIT_IO;
            scheduler();
		}
	}
	res = *vc->buf.in_start;
	vc->buf.in_size--;
	vc->buf.in_start++;
	if(vc->buf.in_start>vc->buf.in_buf+INPUT_BUFFER_SIZE)
		vc->buf.in_start-=INPUT_BUFFER_SIZE;
	return res;
}

/* Discard last character, and return if buffer was empty before operation */
static int buffer_unpush(struct virtual_console *vc)
{
    int is_empty = (vc->buf.in_size == 0);
    if(is_empty)
        return is_empty;
    vc->buf.in_size --;
    return is_empty;
}

unsigned long cons_read(char* buf, unsigned long len)
{
	unsigned int offset;
    buf = check_pointer(buf, SYSCALL_CONS_READ);
	for(offset=0;offset<len;offset++) {
		char c = buffer_pop(
            &virtual_consoles[tab_proc[pid_cour-1].
            file_desc[1].node->inode-100]);
        if(c == '\n')
            return offset;
        buf[offset] = c;
	}
    return len;
}

/* Buffer mode control */
void console_set_buffered_mode(struct virtual_console *vc)
{
    vc->buffered = 1;
}

void console_set_unbuffered_mode(struct virtual_console *vc)
{
    vc->buffered = 0;
}

int console_is_buffered(struct virtual_console *vc)
{
    return vc->buffered;
}

/* Interrupt handler */
void handle_int33(void)
{
    outb(0x20, 0x20);
    int scan = inb(KB_DATA);
    if(0x3b <= scan && scan <= 0x3e) {  /* F1..F4 */
        vc_flip(scan - 0x3b);
        return;
    }
    do_scancode(scan);
}

static void console_scroll(struct virtual_console *vc)
{
    vc->pos -= NB_COL;
    int i;
    for(i=0;i<NB_COL*(NB_ROW-1); i++) {
        vc->mem[i] = vc->mem[i+NB_COL];
    }
    for(;i<NB_COL*NB_ROW;i++) {
        vc->mem[i].c = ' ';
    }
    vc_draw(vc);
}

static void console_write_register(unsigned char reg, int value)
{
    outb(reg,   SCREEN_SEL);
    outb(value, SCREEN_REG);
}

static void console_update_cursor(struct virtual_console *vc)
{
    console_write_register(0x0f,  vc->pos & 0xff);
    console_write_register(0x0e, (vc->pos & 0xff00)>>8);
}

static void console_drawbyte (struct virtual_console *vc, const char c)
{
    switch(c) {
        case '\n':
        case '\r':
        case '\t':
            break;
        case '\b':
            vc->pos--;
            console_drawbyte(vc, ' ');
            break;
        default:
            vc->mem[vc->pos].c    = c;
            vc->mem[vc->pos].attr = vc->current_attr;
            if(vc == vc_current) {
                /* Commit changes to video memory, too */
                video_mem[vc->pos].c    = c;
                video_mem[vc->pos].attr = vc->current_attr;
            }
            break;
    }
}

static void console_putbyte (struct virtual_console *vc, const char c)
{
    console_drawbyte(vc, c);
    switch(c){
    case '\n':
    case '\r':
        vc->pos += NB_COL - (vc->pos % NB_COL);
        break;
    case '\t':
        do {
            vc->pos++;
        } while (vc->pos % 8 != 0);
        break;
    case '\b':
        break;
    default:
        vc->pos++;
    }
    if(vc->pos >= NB_COL*NB_ROW) {
        console_scroll(vc);
        console_drawbyte(vc,c);
    }
    console_update_cursor(vc);
}

void console_putbytes(const char *s, int len)
{
    if (is_in_syscall(SYSCALL_CONS_WRITE)) {
        write(1, s, len);
    }
    else {
        int i;
        for(i=0;i<len;i++) {
            console_putbyte(vc_current, s[i]);
        }
    }
}

void console_clear(struct virtual_console *vc)
{
    int i;
    vc->pos=0;
    for(i=0;i<NB_COL*NB_ROW;i++) {
        vc->mem[i].c = ' ';
        vc->mem[i].attr = vc->current_attr;
    }
    vc_draw(vc);
}

void clear(void)
{
    console_clear(vc_current);
}

void console_init(void)
{
    fill_interrupt(33, KERNEL_CS, int33, 0);
    init_irqs(IRQ(1));

    int i;
    for(i=0;i<VIRTUAL_CONSOLES;i++) {
        vc_init(&virtual_consoles[i]);
        virtual_consoles[i].mem = mem_alloc(
                                NB_ROW*NB_COL*sizeof(struct console_character));
        console_clear(&virtual_consoles[i]);
    }
    vc_current = &virtual_consoles[0];
}

void cons_write(const char* buf, int len)
{
    buf = check_pointer(buf, SYSCALL_CONS_WRITE);
    console_putbytes(buf, len);
}

void cons_echo(int on)
{
    vc_current->echo = on;
}

/* Keyboard handling callback (called by keyboard-glue) */
void keyboard_data(char* str)
{
    if(*str == 127)  *str = '\b';
    if(*str == '\r') *str = '\n';

    if(*str == '\b') {
        if(!buffer_unpush(vc_current) && vc_current->echo) {
            console_putbyte(vc_current, *str);
        }
        return;
    } else {
        buffer_push(vc_current, *str);
    }
    if(vc_current->echo)
        console_putbyte(vc_current,*str);
}

/******************************
 *          Leds              *
 ******************************/
static int console_leds=0;

void console_leds_update(void)
{
    outb(0xed, KB_DATA);
    outb(console_leds, KB_DATA);
}

/* Called by keyboard-glue */
void kbd_leds(unsigned char leds){
    console_leds = leds;
    console_leds_update();
}

/* Coooooool led */
void console_scrolllock_on(void)
{
    console_leds |= 1;
    console_leds_update();
}

void console_scrolllock_off(void)
{
    console_leds &= ~1;
    console_leds_update();
}

void cons_ioctl(int req, void* p)
{
    p = check_pointer(p, SYSCALL_CONS_IOCTL);
    struct cons_pal* pal;
    switch((enum cons_ioctl_req) req){
    case CONS_SETPAL:
        pal = (struct cons_pal*) p;
        outb(0xff,       SCR_PALETTE_MASK);
        outb(pal->index, SCR_PALETTE_WRITE);
        outb(pal->r,     SCR_PALETTE_DATA);
        outb(pal->g,     SCR_PALETTE_DATA);
        outb(pal->b,     SCR_PALETTE_DATA);
        break;
    case CONS_GETPAL:
        pal = (struct cons_pal*) p;
        outb(0xff,       SCR_PALETTE_MASK);
        outb(pal->index, SCR_PALETTE_READ);
        pal->r   =   inb(SCR_PALETTE_DATA);
        pal->g   =   inb(SCR_PALETTE_DATA);
        pal->b   =   inb(SCR_PALETTE_DATA);
        break;
    case CONS_SETATTR:
        vc_current->current_attr = *(int*) p;
        break;
    case CONS_GETATTR:
        *(int*) p = vc_current->current_attr;
        break;
    case CONS_SETBUF:
        vc_current->buffered = *(int*) p;
        break;
    case CONS_GETBUF:
        *(int*) p = vc_current->buffered;
        break;
    }
}

/****** Callbacks for sysfs *******/
static int console_blink_r(fs_node_t *node, unsigned long offset,
        unsigned long len, char *buf)
{
    int attr;
    if(offset == 0) {
        cons_ioctl(CONS_GETATTR, &attr);
        snprintf(buf, 1, "%d", (attr & 0x80)?1:0);
        return 1;
    }
    else return 0;
}
static int console_blink_w(fs_node_t *node, unsigned long offset,
        unsigned long len, const char *buf)
{
    if(len>0)  {/* we have at least one byte to read ! */
        int attr;
        cons_ioctl(CONS_GETATTR, &attr);
        if (buf[0] == '0') {
            attr &= ~0x80;
        } else {
            attr |=  0x80;
        }
        cons_ioctl(CONS_SETATTR, &attr);
        return 1;
    }
    return -1;
}
static int console_fg_r (fs_node_t *node, unsigned long offset,
                                          unsigned long len,   char *buf)
{
    char fgbuf[3];
    int attr;
    cons_ioctl(CONS_GETATTR, &attr);
    int z = sprintf(fgbuf, "%d", attr & 0x0f);
    return memread(fgbuf, z, offset, buf, len);
}
static int console_fg_w (fs_node_t *node, unsigned long offset,
                                          unsigned long len, const char *buf)
{
    if(len>0)  {/* we have at least one byte to read ! */
        int attr;
        cons_ioctl(CONS_GETATTR, &attr);
        attr &= ~0x0f;
        attr |= (buf[0]-'0')&0x0f;
        cons_ioctl(CONS_SETATTR, &attr);
        return 1;
    }
    return -1;
}
static int console_bg_r (fs_node_t *node, unsigned long offset,
                                          unsigned long len,   char *buf)
{
    char bgbuf[3];
    int attr;
    cons_ioctl(CONS_GETATTR, &attr);
    int z = sprintf(bgbuf, "%d", (attr&0x70)>>4);
    return memread(bgbuf, z, offset, buf, len);
}
static int console_bg_w (fs_node_t *node, unsigned long offset,
                                          unsigned long len, const char *buf)
{
    if(len>0)  {/* we have at least one byte to read ! */
        int attr;
        cons_ioctl(CONS_GETATTR, &attr);
        attr &= ~0x70;
        attr |= ((buf[0]-'0')<<4)&0x70;
        cons_ioctl(CONS_SETATTR, &attr);
        return 1;
    }
    return -1;
}
static int console_echo_w(fs_node_t *node, unsigned long offset,
        unsigned long len, const char *buf)
{
    if(len>0)  {/* we have at least one byte to read ! */
        cons_echo(buf[0]-'0');
        return 1;
    }
    return -1;
}

static int console_vc_readdir(struct fs_node* node, struct dirent *dirp, int n)
{
    if (n<0)                     return -1;
    if (n >= VIRTUAL_CONSOLES)   return  0;
    snprintf(dirp->name, FS_NAME_MAX, "%d", n);
    dirp->inode=100+n;
    return 1;
}

static struct fs_node* console_vc_finddir(struct fs_node* node,const char *name)
{
    int vc_n = strtol(name, NULL, 10);
    if(vc_n<0 || vc_n >= VIRTUAL_CONSOLES) {
        return NULL;
    }
    return &vc_fs_node[vc_n];
}

/* Hooked on /dev/vc/X */
static int console_vc_write (struct fs_node *node, unsigned long offset,
    unsigned long len, const char* s)
{
    int i;
    for (i=0;i<len;i++) {
        console_putbyte(&virtual_consoles[node->inode-100], s[i]);
    }
    return len;
}

void console_vfs_init(void)
{
    /* /dev/vc */
    vfs_node_register_callbacks(17, NULL, NULL, NULL, NULL,console_vc_readdir,
        console_vc_finddir);

    /* /dev/vc/N */
    int i;
    for(i=0;i<VIRTUAL_CONSOLES;i++) {
        sprintf(vc_fs_node[i].name, "%d", i);
        vc_fs_node[i].type   = FS_FILE;
        vc_fs_node[i].inode  = 100 + i;
        vc_fs_node[i].write  = console_vc_write;
    }

    /* /sys/console/blink */
    vfs_node_register_callbacks(5, console_blink_r, console_blink_w,
        NULL, NULL, NULL, NULL);

    /* /sys/console/fg */
    vfs_node_register_callbacks(6, console_fg_r, console_fg_w, NULL, NULL,
        NULL, NULL);

    /* /sys/console/bg */
    vfs_node_register_callbacks(7, console_bg_r, console_bg_w, NULL, NULL,
        NULL, NULL);

    /* /sys/console/echo */
    vfs_node_register_callbacks(13, NULL, console_echo_w, NULL, NULL,
        NULL, NULL);

}
