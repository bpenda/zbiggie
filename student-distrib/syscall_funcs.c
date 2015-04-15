#include "syscall_funcs.h"
#include "zbigfs.h"
#include "x86_desc.h"
#include "page.h"
#define MAGIC_NUM 0x464c457f
#define CHAR_NUM    32
#define LAST_INDEX	127 
#define OFFSET 24
#define EXE_HEADER_SZ (OFFSET + sizeof(int))
#define ZERO 0 
#define ONE 1
#define FAIL -1 
#define EXE_LOAD_SZ ( 33 * OFFSET_4M - LOAD_ADDR)

/* - - - - - - - - - - - - - - - - - - - - - - 
    Helper Functions
 - - - - - - - - - - - - - - - - - - - - - - */ 

int syscall_open(const char * name){
    int fd = get_new_fd();
    if (fd == -1) return -1;
    int ret = kopen(get_file(fd), name);
    if (ret) return ret;
    return fd;
}

int min(int a, int b){ return a<b?a:b;}
int syscall_getargs(char * buf, int nbytes){
    int bytes_copied = strncpy(buf, get_current_pcb()->cmdstring, min(nbytes, BUF_SIZE));
    if(bytes_copied < nbytes)
        return -1;
    return 0;
}

int syscall_close(int fd){
    FILE * f = get_file(fd);
    if ((int)(f)==-1) return -1;
    kclose(f);
    return free_fd(fd);
}

void * load_exec_to_mem( const char * fname)
{
 	FILE f;

	char exec[BUF_SIZE]; 
	char args[BUF_SIZE];

    parse_input(fname, exec, args, BUF_SIZE);
    char buf[EXE_HEADER_SZ];
	if (kopen(&f, exec)) { return (void*)FAIL;}
    char * mem = (char *) LOAD_ADDR;
	kread(&f, buf, EXE_HEADER_SZ);

    if(*(int*)buf != MAGIC_NUM) {
        return (void*)FAIL;
    }

    if (setup_new_process()){
        return (void*)ONE;
    }
    memcpy(mem, buf, EXE_HEADER_SZ);
	kread(&f, mem+EXE_HEADER_SZ, EXE_LOAD_SZ);
   
    void * entry = *((void **)(mem+OFFSET));
    strncpy(get_current_pcb()->cmdstring, args, BUF_SIZE);
    return entry;
}

void parse_input(const char * in, char * exec_buf, char * args_buf, int size)
{
    int i = 0;
    int ei = 0;
    int ai = 0;

    args_buf[0] = 0;

    if(in == 0)
        return;

    while(in[i] == ' ')
        i++;

    for(; i < size && in[i] != 0 && in[i] != '\n'; i++, ei++)
    {
        if(in[i] == ' ')
            break;
        exec_buf[ei] = in[i];
    }
    exec_buf[ei] = '\0';

    while(in[i] == ' ')
        i++;

    for(; i < size && in[i] != 0 && in[i] != '\n'; i++, ai++)
    {
        args_buf[ai] = in[i];
    }
    args_buf[ai] = '\0';

}


