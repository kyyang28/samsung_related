
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>


#define MAX_LEN     100

void input_handler(int num)
{
    char data[MAX_LEN];
    int len;

    /* Read and output the contents in the STDIN_FILENO */
    len = read(STDIN_FILENO, &data, MAX_LEN);
    data[len] = 0;
    printf("input available: %s\n", data);
}

int main(int argc, char **argv)
{
    int flags;

    /* 
     * 1. 通过F_SETOWN IO控制命令设置设备文件的拥有者为本进程，这样从设备驱动
     *    发出的信号才能被本进程接收到
     */
    fcntl(STDIN_FILENO, F_SETOWN, getpid());

    /* 2. 通过F_GETFL获得标志 */
    flags = fcntl(STDIN_FILENO, F_GETFL);

    /* 3. 通过F_SETFL IO控制命令设置设备文件支持FASYNC，即异步通知模式 */
    fcntl(STDIN_FILENO, F_SETFL, flags | FASYNC);

    /* 
     * 启动信号机制 
     * 4. 通过signal()函数连接信号和信号处理函数
     */
    signal(SIGIO, input_handler);
    
    while (1);

    return 0;
}


