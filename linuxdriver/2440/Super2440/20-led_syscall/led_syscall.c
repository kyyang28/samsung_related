

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
//#include <sysdep.h>


//#if defined(__thumb__) || defined(__ARM_EABI__)
//#define __NR_SYSCALL_BASE	0
//#else
#define __NR_SYSCALL_BASE	0x900000
//#endif


/* Usage: led("<on | off> <1 | 2 | 3 | 4>", count) */
void led(char *buf, int count)
{
    /* swi */    
    asm ("mov r0, %0\n"   /* save the argment in r0 */
         "mov r1, %1\n"   /* save the argment in r1 */
         "swi %2\n"   /* do the system call */
         :
         : "r"(buf), "r"(count), "i" (__NR_SYSCALL_BASE+371)
         : "r0", "r1");
}

int main(int argc, char **argv)
{
    printf("Charles.Y[app]: call led syscall\n");
    //hello("ARM LINUX", 9);

    led("off all", 10);
    usleep(500000);

    while (1) {        
        led("on 1", 4);
        usleep(500000);
        led("off 1", 4);        
        usleep(500000);
        led("on 2", 4);        
        usleep(500000);
        led("off 2", 4);        
        usleep(500000);
        led("on 3", 4);        
        usleep(500000);
        led("off 3", 4);        
        usleep(500000);
        led("on 4", 4);        
        usleep(500000);
        led("off 4", 4);        
        usleep(500000);        
    }

    return 0;
}


