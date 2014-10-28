
#include "s3c2440.h"


void disable_watchdog(void)
{
    WTCON = 0; 
}


void copy2sdram(void)
{
    unsigned int *pdwSrc  = (unsigned int *)2048;
    unsigned int *pdwDest = (unsigned int *)0x30004000;
    
    while (pdwSrc < (unsigned int *)4096)
    {
        *pdwDest = *pdwSrc;
        pdwDest++;
        pdwSrc++;
    }
}


void create_page_table(void)
{
    unsigned long virtualaddr, physicaladdr;
    unsigned long *mmu_tlb_base = (unsigned long *)MMU_TABLE_BASE;
    
    /*
     * Steppingstone的起始物理地址为0，第一部分程序的起始运行地址也是0，
     * 为了在开启MMU后仍能运行第一部分的程序，
     * 将0～1M的虚拟地址映射到同样的物理地址
     */
    virtualaddr = 0;
    physicaladdr = 0;
#ifdef ENABLE_CACHABLE_AND_BUFFERABLE	// Macro is defined in Makefile
    *(mmu_tlb_base + (virtualaddr >> 20)) = physicaladdr | MMU_SECDESC_WB;
#else
    *(mmu_tlb_base + (virtualaddr >> 20)) = physicaladdr | MMU_SECDESC;
#endif

    /*
     * 0x56000000是GPIO寄存器的起始物理地址，
     * GPFCON和GPFDAT这两个寄存器的物理地址0x56000050、0x56000054，
     * GPGCON和GPGDAT这两个寄存器的物理地址0x56000060、0x56000064，
     * 为了在第二部分程序中能以地址0xD6000050、0xD6000054来操作GPFCON、GPFDAT，
     * 以及在第二部分程序中能以地址0xD6000060、0xD6000064来操作GPGCON、GPGDAT，
     * 把从0xD6000000开始的1M虚拟地址空间映射到从0x56000000开始的1M物理地址空间
     */
    virtualaddr = (0x56000000 + 0x80000000);    // 0xD6000000
    physicaladdr = 0x56000000;
    *(mmu_tlb_base + (virtualaddr >> 20)) = physicaladdr | MMU_SECDESC;

    /*
     * SDRAM的物理地址范围是0x30000000～0x33FFFFFF，
     * 将虚拟地址0xB0000000～0xB3FFFFFF映射到物理地址0x30000000～0x33FFFFFF上，
     * 总共64M，涉及64个段描述符
     */
    virtualaddr = (0x30000000 + 0x80000000);    // 0xB0000000
    physicaladdr = 0x30000000;
    while (virtualaddr < 0xB4000000)
    {
#ifdef ENABLE_CACHABLE_AND_BUFFERABLE
	    *(mmu_tlb_base + (virtualaddr >> 20)) = physicaladdr | MMU_SECDESC_WB;
#else
        *(mmu_tlb_base + (virtualaddr >> 20)) = physicaladdr | MMU_SECDESC;
#endif
	    virtualaddr += MMU_SECTION_SIZE;         // 0x100000;
        physicaladdr += MMU_SECTION_SIZE;        // 0x100000;
    }
}


void mmu_init(void)
{
    unsigned long ttb = 0x30000000;

// 嵌入汇编：LINUX内核完全注释
__asm__(
    "mov    r0, #0\n"
    "mcr    p15, 0, r0, c7, c7, 0\n"    /* 使无效ICaches和DCaches */
    
    "mcr    p15, 0, r0, c7, c10, 4\n"   /* drain write buffer on v4 */
    "mcr    p15, 0, r0, c8, c7, 0\n"    /* 使无效指令、数据TLB */
    
    "mov    r4, %0\n"                   /* r4 = 页表基址 */
    "mcr    p15, 0, r4, c2, c0, 0\n"    /* 设置页表基址寄存器 */
    
    "mvn    r0, #0\n"                   
    "mcr    p15, 0, r0, c3, c0, 0\n"    /* 域访问控制寄存器设为0xFFFFFFFF，
                                         * 不进行权限检查 
                                         */    
    /* 
     * 对于控制寄存器，先读出其值，在这基础上修改感兴趣的位，
     * 然后再写入
     */
    "mrc    p15, 0, r0, c1, c0, 0\n"    /* 读出控制寄存器的值 */
    
    /* 控制寄存器的低16位含义为：.RVI ..RS B... .CAM
     * R : 表示换出Cache中的条目时使用的算法，
     *     0 = Random replacement；1 = Round robin replacement
     * V : 表示异常向量表所在的位置，
     *     0 = Low addresses = 0x00000000；1 = High addresses = 0xFFFF0000
     * I : 0 = 关闭ICaches；1 = 开启ICaches
     * R、S : 用来与页表中的描述符一起确定内存的访问权限
     * B : 0 = CPU为小字节序；1 = CPU为大字节序
     * C : 0 = 关闭DCaches；1 = 开启DCaches
     * A : 0 = 数据访问时不进行地址对齐检查；1 = 数据访问时进行地址对齐检查
     * M : 0 = 关闭MMU；1 = 开启MMU
     */
    
    /*  
     * 先清除不需要的位，往下若需要则重新设置它们    
     */
                                        /* .RVI ..RS B... .CAM */ 
    "bic    r0, r0, #0x3000\n"          /* ..11 .... .... .... 清除V、I位 */
    "bic    r0, r0, #0x0300\n"          /* .... ..11 .... .... 清除R、S位 */
    "bic    r0, r0, #0x0087\n"          /* .... .... 1... .111 清除B/C/A/M */

    /*
     * 设置需要的位
     */
    "orr    r0, r0, #0x0002\n"          /* .... .... .... ..1. 开启对齐检查 */
    "orr    r0, r0, #0x0004\n"          /* .... .... .... .1.. 开启DCaches */
    "orr    r0, r0, #0x1000\n"          /* ...1 .... .... .... 开启ICaches */
    "orr    r0, r0, #0x0001\n"          /* .... .... .... ...1 使能MMU */
    
    "mcr    p15, 0, r0, c1, c0, 0\n"    /* 将修改的值写入控制寄存器 */
    : /* 无输出 */
    : "r" (ttb) );
}


