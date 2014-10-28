
#include "ledlib.h"

//#define __led_on_swi_no             (1)
//#define __led_off_swi_no            (2)
//#define __led_all_on_swi_no         (3)
//#define __led_all_off_swi_no        (4)

void led_on(int num)
{
    __asm__ (
        //"mov r0, %[num]\n"
	"swi 1\n"
	//:[num]"=r"(num)
    );
}

void led_off(int num)
{
    __asm__ (
        //"mov r0, %[num]\n"
        "swi 2\n"
	//:[num]"=r"(num)
    );
}

void led_all_on(void)
{
    __asm__ (
        "swi 3\n"
    );
}

void led_all_off(void)
{
    __asm__ (
        "swi 4\n"
    );
}

