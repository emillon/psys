#include "shell.h"
#include "syscalls.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include "console.h"
#include "tests.h"
#include "mem.h"

#define CMD_BUF_SIZE 512

typedef void (*shell_callback)(int argc, char** argv);

#define SHELL_MAX_CMD 32
static struct {
	char* cmd;
	char* description;
	shell_callback cb;
} shell_association[SHELL_MAX_CMD];
static int shell_n_cmd = 0;
static int shell_max_cmd_len = 0;

int shell_cmd_mtx = -1;
static int shell_init_done = 0;

static void shell_register_command(char* cmd, char* description, 
				   shell_callback cb)
{
    shell_association[shell_n_cmd].cmd = cmd;
    shell_association[shell_n_cmd].description = description;
    shell_association[shell_n_cmd].cb = cb;
    int len = strlen(cmd);
    if(len>shell_max_cmd_len) shell_max_cmd_len = len;
    shell_n_cmd++;

    shell_init_done = 1;
}

static void shell_execute_cmd(char* cmd)
{
    if(!strcmp(cmd, "")) return;

    int    argc = 0;
    char*  argv[16]; // max number of args
    char* ptr = cmd;

    while(1) {
        for (;*ptr != '\0' && isspace(*ptr);ptr++);
        if(*ptr == '\0') break;
        // non-whitespace : beginning of arg
        argv[argc++]=ptr;
        for (;*ptr!='\0' && !isspace(*ptr);ptr++);
        // end of arg
        if(*ptr == '\0') break;
        *ptr++='\0';
    }

    int matches[shell_n_cmd];
    for(int i=0;i<shell_n_cmd;i++){
        matches[i]=1;
    }
    int match = -1;
    for(int depth=strlen(argv[0]);depth<=shell_max_cmd_len;depth++) {
        int n_matches = 0;
        for(int i=0;i<shell_n_cmd;i++) {
            if(!strncmp(shell_association[i].cmd, argv[0], depth)) {
                n_matches++;
                match = i;
            } else {
                matches[i] = 0;
            }
        }
        if(n_matches==1) {
            (* shell_association[match].cb)(argc, argv);
            return;
        } else if(n_matches>1) {
            printf("Ambiguous command. Did you mean : ");
            for(int j=0;j<shell_n_cmd;j++) {
                if(matches[j]==0) continue;
                printf("%s ", shell_association[j].cmd);
            }
            printf ("\n");
            return;
        }
    }
    printf("%s : unknown command\n", cmd);
}

/****************************************************************************
 *                                                                          *
 *                                stdio                                     *
 *                                                                          *
 ****************************************************************************/

/* Read a line upto len characters */
/* It converts the final \n to \0 */
int readline(char* buf, unsigned long len)
{
    cons_read(buf, len);
    for(;*buf!='\n';buf++);
    *buf='\0';
    return len;
}

void putc(char c){
    printf("%c", c);
}

/* Return the integer represented by s */
int atoi(const char* s)
{
    int n = 0;
    for(;*s!='\0';s++){
        n = 10*n + (*s-'0');
    }
    return n;
}

/**************************************** 
 *           Shell builtins             * 
 ****************************************/
static void shell_reboot(int argc, char** argv)
{
    (void) argc; (void) argv;
	reboot();
}

static void shell_help(int argc, char** argv)
{
    int i;
    if(argc==2) {
        char* desc = NULL;
        for(i=0;i<shell_n_cmd;i++)
            if(!strcmp(shell_association[i].cmd, argv[1])) {
                desc = shell_association[i].description;
                break;
            }
        if(desc == NULL) {
            printf("Unknown command.\n");
        }
        printf("%s - %s\n", argv[0], desc);
        return;
    }
    printf("Available commands :\n");
    for(i=0;i<shell_n_cmd;i++) {
        printf("%-16s %s\n", shell_association[i].cmd, 
                shell_association[i].description);
    }
}

static void cpuid_query(unsigned long query, unsigned long* regs)
{
        regs[0] = query;
	__asm__ __volatile__ (
		  "cpuid"
          : "=a" (regs[0]), /* outputs */
            "=b" (regs[1]),
            "=c" (regs[2]),
            "=d" (regs[3])
          : "a"  (regs[0]) /* input */
          );
}

static void shell_cpuid (int argc, char** argv)
{
    (void) argc; (void) argv;
    unsigned long regs[4];
    cpuid_query(0x00000000, regs);
    int i;
    printf("CPU Vendor ID          <");
    for(i=0;i<32;i+=8) putc((regs[1] & (0xff << i)) >> i);
    for(i=0;i<32;i+=8) putc((regs[3] & (0xff << i)) >> i);
    for(i=0;i<32;i+=8) putc((regs[2] & (0xff << i)) >> i);
    printf(">\n");

    printf("Processor Brand String <");
    cpuid_query(0x80000002, regs);
    for(i=0;i<32;i+=8) putc((regs[0] & (0xff << i)) >> i);
    for(i=0;i<32;i+=8) putc((regs[1] & (0xff << i)) >> i);
    for(i=0;i<32;i+=8) putc((regs[2] & (0xff << i)) >> i);
    for(i=0;i<32;i+=8) putc((regs[3] & (0xff << i)) >> i);
    cpuid_query(0x80000003, regs);
    for(i=0;i<32;i+=8) putc((regs[0] & (0xff << i)) >> i);
    for(i=0;i<32;i+=8) putc((regs[1] & (0xff << i)) >> i);
    for(i=0;i<32;i+=8) putc((regs[2] & (0xff << i)) >> i);
    for(i=0;i<32;i+=8) putc((regs[3] & (0xff << i)) >> i);
    cpuid_query(0x80000004, regs);
    for(i=0;i<32;i+=8) putc((regs[0] & (0xff << i)) >> i);
    for(i=0;i<32;i+=8) putc((regs[1] & (0xff << i)) >> i);
    for(i=0;i<32;i+=8) putc((regs[2] & (0xff << i)) >> i);
    for(i=0;i<32;i+=8) putc((regs[3] & (0xff << i)) >> i);

    printf(">\n");
}

static void shell_palset(int argc, char** argv)
{
    if(argc != 5){
        printf("Syntax : palset n r g b\n");
        return;
    }
    struct cons_pal palette = {
        .index = atoi(argv[1]),
        .r     = atoi(argv[2]),
        .g     = atoi(argv[3]),
        .b     = atoi(argv[4])
    };
    cons_ioctl(CONS_SETPAL, &palette);
    return;
}

struct color_theme {
    const char* name;
    struct cons_pal bg;
    struct cons_pal fg;
};

static void shell_colortheme(int argc, char** argv)
{
    static struct color_theme themes[] = {
        {"amstrad",  {0,  0,  0, 63}, {7, 63, 63,  0}},
        {"white",    {0, 63, 63, 63}, {7,  0,  0,  0}},
        {"sky",      {0,  0,  0, 63}, {7, 63, 63, 63}},
        {"default",  {0,  0,  0,  0}, {7, 32, 32, 32}}
    };
    const int n_themes = sizeof(themes)/sizeof(struct color_theme);
    if (argc!=2){
        printf("Usage : colortheme <name>\n");
        int i;
        for(i=0;i<n_themes;i++) {
            printf("%s ", themes[i].name);
        }
    printf("\n");

        return;
    }
    int i;
    for(i = 0;i<n_themes;i++) {
        if(!strcmp(themes[i].name, argv[1])) {
            cons_ioctl(CONS_SETPAL, &themes[i].fg);
            cons_ioctl(CONS_SETPAL, &themes[i].bg);
            return;
        }
    }
    printf("No such theme, available themes are :\n");
    for(i=0;i<n_themes;i++) {
        printf("%s ", themes[i].name);
    }
    printf("\n");
}

static void shell_tests(int argc, char** argv)
{
    (void) argc; (void) argv;
    run_test_suite();
}

static void shell_ps(int argc, char** argv)
{
    (void) argc; (void) argv;
    context_ps();
}

static void shell_sinfo(int argc, char** argv)
{
    (void) argc; (void) argv;
    sem_info();
}

static void shell_piano(int argc, char** argv)
{
    int buffered = 0;
    if (argc>1)
        buffered = 1;
    printf("\n");
    printf("Pimp My Piano\n");
    printf("\n");
    printf("  z   e       t   y   u       o   p    \n");
    printf("                                       \n");
    printf("q   s   d   f   g   h   j   k   l   m  \n");
    printf("\n");
    printf("Press n to quit\n");
    cons_echo(0);
    static const char notes[256] = {
        [(int)'q'] = 100, /* 100 + semitones */
        [(int)'z'] = 101,
        [(int)'s'] = 102,
        [(int)'e'] = 103,
        [(int)'d'] = 104,
        [(int)'f'] = 105,
        [(int)'t'] = 106,
        [(int)'g'] = 107,
        [(int)'y'] = 108,
        [(int)'h'] = 109,
        [(int)'u'] = 110,
        [(int)'j'] = 111,
        [(int)'k'] = 112,
        [(int)'o'] = 113,
        [(int)'l'] = 114,
        [(int)'p'] = 115,
        [(int)'m'] = 116,
        [(int)'n'] =  -1 /* quit */
    };
    cons_ioctl(CONS_SETBUF, &buffered);
    while(1) {
        char c;
        cons_read(&c, 1);
        if(notes[(int)c]==-1) break;
        if(notes[(int)c]== 0) continue;
        /* else, play note */
        buzzer_beep(notes[(int)c]-100, 200);
    }
    buffered=1;
    cons_ioctl(CONS_SETBUF, &buffered);
    cons_echo(1);
    printf("\n");
}

static void shell_clear(int argc, char** argv)
{
    (void) argc; (void) argv;
    if(argc!=1){
        printf("Usage : clear\n");
        return;
    }
    clear();
}

static void shell_ls(int argc, char** argv)
{
    char * name;
    if(argc>2) {
        printf("Usage : ls <filename>\n");
        return;
    }
    if(argc == 2) {
        name = argv[1];
    } else {
        name = ".";
    }
        
    int fd = open(name);
    if(fd<0) {
        printf("No such file or directory\n");
        return;
    }
    struct stat st;
    if(fstat(fd, &st)<0) {
        printf("Error in fstat\n");
        return;
    }
    printf("Name : %s\n", st.name);
    printf("Inode number : %d\n", st.inode);
    if(st.type == FS_FILE) {
        printf("Type : file\n");
    } else if(st.type == FS_DIR) {
        printf("Type : directory\n"); 
        printf("Contents :\n");
        int n = 0;
        while(1) {
            struct dirent dir;
            int z = readdir(fd, &dir, n);
            if(z ==  0) break;
            if(z == -1) {
                printf("Error reading directory entry #%d\n", n);
                return;
            }
            printf("%4d %s\n",
                dir.inode, 
                dir.name);
            n++;
        }
    } else {
        printf("Type : <unknown>\n");
    } 
    if(close(fd)<0) {
        printf("Error closing file\n");
        return;
    }
}
 
static void shell_pwd(int argc, char** argv)
{ 
    char filename[FS_NAME_MAX];
    if(getcwd(filename, FS_NAME_MAX) != NULL) { 
        printf("WD : %s\n", filename);
    } else {
        printf("Error getting current working directory.\n");
    }
 }

static void shell_cwd(int argc, char** argv)
{
    if(argc!=2) {
        printf("Usage : cd <directory name>\n");
    }
    if(chdir(argv[1]) == 0) {
        printf("OK, CWD = %s\n", argv[1]);
    } else {
        printf("Cannot chdir()\n");
    }
}

static void shell_cat(int argc, char** argv)
{
    int max_count;
    char* name;
    if(argc>3 || argc == 1) {
        printf("Usage : cat <filename> [max len]\n");
        return;
    }
    name = argv[1];
    if(argc == 3) {
        max_count = atoi(argv[2]);
    } else {
        max_count = 1<<16;
    }
    int fd;
    char buf[64];
    if((fd = open(name))<0) {
        printf("Error opening file\n");
        return;
    }
    while(1) {
        int i, z = read(fd, buf, 64);
        if(z==-1) {
            printf("Error reading file\n");
            break;
        }
        if(z==0) {
            /* EOF */
            break;
        }
        for (i=0;i<z;i++) {
            printf("%c", buf[i]);
        }
        max_count -= z;
        if(max_count <= 0)
            break;
    }

    if((close(fd))!=0) {
        printf("Error closing file\n");
        return;
    }
    printf("\n");
}

static void shell_write_file (int argc, char** argv)
{
    int fd;
    if(argc!=3) {
        printf("Usage : write <filename> <string>\n");
        return;
    }
    if((fd = open(argv[1]))<0) {
        printf("Error opening file\n");
        return;
    }
    int z = write(fd, argv[2], strlen(argv[2]));
    if(z==-1) {
        printf("Error writing to file\n");
    } else if (z==0) {
        printf("Error : EOF\n");
    }
    if(close(fd)!=0) {
        printf("Error closing file\n");
    }
}

static void shell_debug (int argc, char** argv)
{
    __asm__ __volatile__("int $3");
}

static void shell_exec(int argc, char** argv)
{
    if(argc!=2) {
        printf("Usage : exec <filename>");
    }
    int fd;
    if((fd = open(argv[1]))<0) {
        printf("Error opening file\n");
        return;
    }
    struct stat s;
    if(fstat(fd,&s)!=0) {
        printf("Error : cannot fstat\n");
        goto close_file;
    }
    const size_t size = s.size;
    char* program;
    if((program = mem_alloc(size)) == NULL) {
        printf("Failed to allocate %d bytes\n", size);
    }

    size_t offset=0;
    while(1) {
        size_t z = read(fd, program+offset, 64);
        if (z==-1) {
            printf("Error writing\n");
            goto free_mem;
        } else if (z==0) {
            /* EOF */
            break;
        }
        offset += z;
        printf("Read %d bytes\n", z);
    }
    int retval;
    int pid = start((void*)program, STACK_SIZE, 3, s.name, NULL);
    waitpid(pid, &retval);
    printf("%d\n", retval);
free_mem:
    mem_free(program, size);
close_file:
    if(close(fd)!=0) {
        printf("Error closing file\n");
    }
}

static void shell_prog(int argc, char** argv)
{
    int retval, pid = start((void*)0x20000000, STACK_SIZE, 3, "Program", NULL);
    waitpid(pid, &retval);
    printf("%d\n", retval);
}

static int bad_process(void*p)
{
    *(char*)0=1;
    return 3;
}

static void shell_fault(int argc, char** argv)
{
    int pid = start(bad_process, STACK_SIZE, 3, "Bad process", (void*) 0);
    printf("Waiting for %d...\n", pid);
    waitpid(pid, NULL);
}

int shell_start (void* p)
{
    (void) p;
    char cmd[CMD_BUF_SIZE];
    /* Critical section */
    wait(shell_cmd_mtx);
    if (!shell_init_done) {

	shell_register_command("bad", "Launch a bad process", shell_fault);
	shell_register_command("prog", "Jump to program space", shell_prog);
	shell_register_command("exec", "Execute a file", shell_exec);
	shell_register_command("debug", "Call debugger", shell_debug);
	shell_register_command("write", "Write in a file",shell_write_file);
	shell_register_command("cat", "Display a file contents",shell_cat);
	shell_register_command("cd",  "Change working directory",shell_cwd);
	shell_register_command("pwd", "Print working directory",shell_pwd);
	shell_register_command("ls", "List files",shell_ls);
	shell_register_command("clear", "Clear screen",shell_clear);
	shell_register_command("colortheme", "Change color theme",shell_colortheme);
	shell_register_command("piano",      "Pimp My Piano", shell_piano);
	shell_register_command("sinfo",  "display semaphore info", shell_sinfo);
	shell_register_command("ps",     "list processes", shell_ps);
	shell_register_command("tests",  "run test suite", shell_tests);
	shell_register_command("palset", "setup the VGA palette", shell_palset);
	shell_register_command("reboot", "reboot the computer", shell_reboot);
	shell_register_command("cpuid",  "gather cpu info",     shell_cpuid);
	shell_register_command("help",   "show a friendly help screen", shell_help);
    }

    signal(shell_cmd_mtx);

    while(1) {
        char buf[32];
        getcwd(buf, 32);
        printf(" [ %s ] %% ", buf);
        int len = cons_read(cmd, CMD_BUF_SIZE);
        cmd[len]='\0';
        shell_execute_cmd(cmd);
    }
    return 0;
}

