
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
     * 1. ͨ��F_SETOWN IO�������������豸�ļ���ӵ����Ϊ�����̣��������豸����
     *    �������źŲ��ܱ������̽��յ�
     */
    fcntl(STDIN_FILENO, F_SETOWN, getpid());

    /* 2. ͨ��F_GETFL��ñ�־ */
    flags = fcntl(STDIN_FILENO, F_GETFL);

    /* 3. ͨ��F_SETFL IO�������������豸�ļ�֧��FASYNC�����첽֪ͨģʽ */
    fcntl(STDIN_FILENO, F_SETFL, flags | FASYNC);

    /* 
     * �����źŻ��� 
     * 4. ͨ��signal()���������źź��źŴ�����
     */
    signal(SIGIO, input_handler);
    
    while (1);

    return 0;
}


