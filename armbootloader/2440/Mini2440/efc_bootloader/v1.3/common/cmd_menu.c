
#include <stdio.h>

extern int run_command(char *cmd);
extern char awaitkey(unsigned long dly);
extern void led_test(void);
extern void buttons_test(void);

static void displayParamsMenuUsage(void)
{
    printf("\r\n");
    printf("+---------------------------------------------+\r\n");
    printf("|        Pete&Charles Bootloader_2012.09      |\r\n");
    printf("+---------------------------------------------+\r\n");
    printf("\r\n");

    printf("[b] Boot the linux kernel\r\n");
    printf("[g] run a program at the specific address\r\n");
    printf("[h] hardware testing programs\r\n");
    printf("[l] load a program using kermit protocol\r\n");
    printf("[r] Reset the bootloader\r\n");
    printf("[q] Quit to shell\r\n\r\n");
    printf("Enter your selection: ");
}

void showParamsMenu(void) 
{
    char c;
    
    while (1) {
        displayParamsMenuUsage();
        c = awaitkey(-1);
        printf("%c\r\n", c);

        switch (c) {
            case 'b':
                run_command("boot");
                break;
                
            case 'g':
                run_command("go");
                break;

            case 'h':
                run_command("hwtest");
                break;

            case 'l':
                run_command("loadb");
                break;

            case 'r':
                run_command("reset");
                break;

            case 'q':                
                printf("\r\n");
                return;
                break;

            default:
                displayParamsMenuUsage();
                break;
        }
    }
}


void displayHWTestMenuUsage(void)
{
    printf("\r\n");
    printf("+---------------------------------------------+\r\n");
    printf("|    Pete&Charles Hardware Testing Programs   |\r\n");
    printf("+---------------------------------------------+\r\n");
    printf("\r\n");

    printf("[l] leds test\r\n");
    printf("[b] buttons_led test\r\n");
    printf("[w] watchdog test\r\n");
    printf("[i] interrupt test\r\n");
    printf("[t] timer test\r\n");
    printf("[q] Quit to shell\r\n\r\n");
    printf("Enter your selection: ");
}

void showHWTestMenu(void) 
{
    char c;
    
    while (1) {
        displayHWTestMenuUsage();
        c = awaitkey(-1);
        printf("%c\r\n", c);

        switch (c) {
            case 'l':
                printf("\r\n@@@@@@@@@@@@@@@@ leds test @@@@@@@@@@@@@@@@\r\n");
                printf("Press 'z' or 'Z' to quit the led test!\r\n");
                led_test();
                break;
                
            case 'b':
                printf("\r\n@@@@@@@@@@@@@@@@ buttons_led test @@@@@@@@@@@@@@@@\r\n");
                printf("Press 'z' or 'Z' to quit the led test!\r\n");
                buttons_test();
                break;

            case 'w':
                printf("watchdog test\r\n");
                break;

            case 'i':
                printf("interrupt test\r\n");
                break;

            case 't':
                printf("timer test\r\n");
                break;

            case 'q':                
                printf("\r\n");
                return;
                break;

            default:
                displayHWTestMenuUsage();
                break;
        }
    }
}

int do_menu(int argc, char *argv[])
{
    showParamsMenu();
    return 0;
}

int do_hwtest(int argc, char *argv[])
{
    showHWTestMenu();
    return 0;
}


