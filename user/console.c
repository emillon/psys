#include "console.h"
#include "syscalls.h"

void console_putbytes(const char*s, int len)
{
    cons_write(s,len);
}
